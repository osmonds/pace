#include "zend.h"

#include <selinux/selinux.h>
#include <selinux/context.h>
#include <syslog.h>


static int
log_callback (int type, const char *fmt, ...)
{
        int audit_fd;
        va_list ap;
        va_start(ap, fmt);
#ifdef HAVE_LIBAUDIT
        audit_fd = audit_open();

        if(audit_fd >= 0){
                char * buf;

                if(vasprintf (&buf, fmt, ap) <0)
                        return 0;
                audit_log_user_avc_message(audit_fd, AUDIT_USER_AVC, buf, NULL, NULL,
                                                NULL, 0);
                audit_close(audit_fd);
                free(buf);
                return 0;
        }
#endif
        vsyslog(LOG_USER | LOG_INFO, fmt, ap);
        va_end(ap);
        return 0;
}

int php_execute_check_selinux(zend_file_handle *file_handle, int type)
{
	security_context_t current_context;
	security_context_t file_context;
	union selinux_callback old_callback;

	old_callback = selinux_get_callback(SELINUX_CB_LOG);
	selinux_set_callback(SELINUX_CB_LOG, (union selinux_callback) &log_callback);
	
	if(getcon(&current_context)){
		zend_error(E_ERROR, "getcon() failed");
	}

	int ret = lgetfilecon(file_handle->filename, &file_context);
	if(ret == -1){
		zend_error(E_ERROR, "getfilecon() failed:%s,%s", file_handle->filename, strerror(errno));
	}
	char * class = "file";
	char * perm_list = "execute";
	
	char * audit_msg;
	
	//int ret;
	ret = selinux_check_access(current_context, file_context,
							class, perm_list,
							NULL);
	zend_error(E_WARNING, "current:%s, file:%s", current_context, file_context);
	
	selinux_set_callback(SELINUX_CB_LOG, old_callback);
	if(ret == -1){
		zend_error(E_WARNING, "permission denied:%s, to %s[%s].", current_context, file_context, file_handle->filename);
		return -1;
	}
	return 0;
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
