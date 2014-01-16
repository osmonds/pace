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
    //zend_error(E_WARNING, "!!!!!!!!!!!!!!!!!!!,audit_fd:%d",audit_fd);

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

int php_execute_check_selinux(zend_file_handle *file_handle, int type)
{
	security_context_t current_context;
	security_context_t file_context;
	union selinux_callback old_callback;
#if 0 
	int audit_fd = audit_open();
	if (audit_fd < 0 && !(errno == EINVAL || errno == EPROTONOSUPPORT ||
                           errno == EAFNOSUPPORT)) {
         /* The above error codes are only given when the kernel doesn't
          * have audit compiled in. */
         zend_error(E_WARNING, "Error - unable to connect to audit system\n");
    }
	zend_error(E_WARNING, "!!!!!!!!!!!!!audit_fd:%d", audit_fd);
#endif
	old_callback = selinux_get_callback(SELINUX_CB_LOG);
	selinux_set_callback(SELINUX_CB_LOG, (union selinux_callback) &log_callback);
	
	if(getcon(&current_context)){
		zend_error(E_ERROR, "getcon() failed");
	}

	int ret = getfilecon(file_handle->filename, &file_context);
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
	char * class = "file";
	char * perm_list = "execute";
	
	char * audit_msg;
#if 0	
	//int ret;
	ret = selinux_check_access(current_context, file_context,
							class, perm_list,
							NULL);
#endif
	unsigned int access = FILE__EXECUTE;
	struct av_decision avd;
	security_class_t tclass;
	tclass = string_to_security_class("file");
	access_vector_t av_perm;
	av_perm = string_to_av_perm(tclass, "execute");
	ret = security_compute_av(current_context, file_context,
				tclass,
				av_perm,
				&avd);
//    zend_error(E_WARNING, "ret:[%d],errno:[%d]and:[%x]allowed[%x]av_perm[%x]tclass[%x]tcontext:%s]filename[%s]",ret, errno, av_perm & avd.allowed, avd.allowed, av_perm, tclass, file_context, file_handle->filename);
	if((ret == 0) && ((av_perm & avd.allowed) == av_perm )){
		ret = 0;
	}else{
		ret = -1;
	}	
	freecon(file_context);
				
//	zend_error(E_WARNING, "current:%s, file:%s", current_context, file_context);
	
//	selinux_set_callback(SELINUX_CB_LOG, old_callback);
	if(ret == -1){
		zend_error(E_WARNING, "permission denied:current:[%s], to file_context[%s][%s].", current_context, file_context, file_handle->filename);
		return -1;
	}
skip_check:
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
