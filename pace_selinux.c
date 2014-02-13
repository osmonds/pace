#include "zend.h"

#include <selinux/avc.h>
#include <libaudit.h>
#include <selinux/selinux.h>
#include <selinux/context.h>
#include <syslog.h>
#include <errno.h>

#include <selinux/flask.h>
#include <selinux/av_permissions.h>


static int
log_callback (int type, const char *fmt, ...)
{
    int audit_fd;
	
    va_list ap;
    va_start(ap, fmt);
    audit_fd = audit_open();
	
	if (audit_fd < 0 && !(errno == EINVAL || errno == EPROTONOSUPPORT ||
                           errno == EAFNOSUPPORT)) {
         /* The above error codes are only given when the kernel doesn't
          * have audit compiled in. */
         zend_error(E_WARNING, "Error - unable to connect to audit system\n");
    }

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
    vsyslog(LOG_USER | LOG_INFO, fmt, ap);
    va_end(ap);
    return 0;
}

/*
 * This callback (SELINUX_CB_AUDIT) is invoked whenever avc_has_perm calls
 * avc_audit with the auditdata parameter. It becomes the 'msg=' entry in the
 * audit log. This callback is optional - use only if additional AVC info needs
 * to be logged.
 */
static int
audit_callback(void *auditdata,
                    security_class_t class,
                    char *msgbuf,
                    size_t msgbufsize)
{
	/******************* Do your processing here *****************/
    /*
	* The auditdata buffer was updated just before calling avc_has_perm()
    * The snprintf function will return a negative value.
    */
    return snprintf(msgbuf, msgbufsize, "%s", (char *)auditdata);
}

#define CLASS "file"
#define PERM_LIST "execute"
#define  AUDIT_MSG_HEAD "PACE:permissin denied for interpretive execute:"

int php_execute_check_selinux(zend_file_handle *file_handle, int type)
{
	security_context_t current_context;
	security_context_t file_context;
	int audit_msg_len;
	char * audit_msg;
	int ret = 0;
	union selinux_callback old_log_callback, old_audit_callback;

	old_log_callback = selinux_get_callback(SELINUX_CB_LOG);
	old_audit_callback = selinux_get_callback(SELINUX_CB_AUDIT);
	selinux_set_callback(SELINUX_CB_LOG, (union selinux_callback) &log_callback);
    selinux_set_callback(SELINUX_CB_AUDIT, (union selinux_callback)&audit_callback);
	
	if(getcon(&current_context)){
		zend_error(E_ERROR, "getcon() failed");
	}

	ret = getfilecon(file_handle->filename, &file_context);
	/*
 	 * Security check should ignore the ENOENT(No such file or directory) error.
 	 * The basic zend engine will handle this error according to the application's code.
 	 */
	if(ret == -1){
		if (errno == ENOENT){
			goto skip_check;
		}
		zend_error(E_ERROR, "getfilecon() failed:%s,%s,%d", file_handle->filename, strerror(errno), errno);
	}

	audit_msg_len = strlen(AUDIT_MSG_HEAD) + strlen(file_handle->filename) + 1;	
	audit_msg = (char*)malloc(audit_msg_len);
	snprintf(audit_msg, audit_msg_len, "%s%s", AUDIT_MSG_HEAD, file_handle->filename);
	audit_msg[audit_msg_len-1] = '\0';	

	ret = selinux_check_access(current_context, file_context,
							CLASS, PERM_LIST,
							audit_msg);
	if( ret ){
		if(errno == EACCES){
			zend_error(E_WARNING, "permission denied:current:[%s], to file :%s with file_context [%s].", current_context, file_handle->filename, file_context);
			ret = -1;
		}else{
			ret = 0;
			zend_error(E_WARNING, "selinux_check_access ERROR:%s",  strerror(errno));
		}	
	}else{
		ret = 0;
	}
	freecon(current_context);
	freecon(file_context);
	free(audit_msg);
	selinux_set_callback(SELINUX_CB_LOG, old_log_callback);
	selinux_set_callback(SELINUX_CB_AUDIT, old_audit_callback);
skip_check:
	return ret;
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
