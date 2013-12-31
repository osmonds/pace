/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2013 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Osmond Sun                                                   |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifndef PHP_PACE_H
#define PHP_PACE_H

#define PACE_NAME		"pace"
#define PACE_VERSION "0.1.0" /* Replace with version number for your extension */
#define PACE_AUTHOR		"Omsond Sun"
#define PACE_COPYRIGHT	"Copyright (c) 2013-2014 by Osmond Sun"
#define PACE_COPYRIGHT_SHORT "Copyright (c) 2013-2014"
#define PACE_URL	"http://github.com"
#define PACE_URL_FAQ "httpd://github.com"


extern zend_module_entry pace_module_entry;
#define phpext_pace_ptr &pace_module_entry


#ifdef PHP_WIN32
#	define PHP_PACE_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_PACE_API __attribute__ ((visibility("default")))
#else
#	define PHP_PACE_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif


typedef struct _zend_compile_args {
  zend_file_handle* file_handle;
  int type;
#ifdef ZTS
  TSRMLS_D;
#endif
} zend_compile_args;

typedef struct _zend_compile_retval {
  zend_op_array * op_array;
  int bailout;
} zend_compile_retval;

typedef struct _zend_execute_args {
  zend_op_array * op_array;
  sigset_t * sigmask;
#ifdef ZTS
  TSRMLS_D;
#endif
} zend_execute_args;

typedef struct _zend_execute_retval {
  int bailout;
} zend_execute_retval;




PHP_MINIT_FUNCTION(pace);
PHP_MSHUTDOWN_FUNCTION(pace);
PHP_RINIT_FUNCTION(pace);
PHP_RSHUTDOWN_FUNCTION(pace);
PHP_MINFO_FUNCTION(pace);

//PHP_FUNCTION(confirm_pace_compiled);	/* For testing, remove later. */

/* 
  	Declare any global variables you may need between the BEGIN
	and END macros here:     

*/
ZEND_BEGIN_MODULE_GLOBALS(pace)
	char *global_value;
	char *global_string;
ZEND_END_MODULE_GLOBALS(pace)

/* In every utility function you add that needs to use variables 
   in php_pace_globals, call TSRMLS_FETCH(); after declaring other 
   variables used by that function, or better yet, pass in TSRMLS_CC
   after the last function argument and declare your utility function
   with TSRMLS_DC after the last declared argument.  Always refer to
   the globals in your function as PACE_G(variable).  You are 
   encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/

#ifdef ZTS
#define PACE_G(v) TSRMG(pace_globals_id, zend_pace_globals *, v)
#else
#define PACE_G(v) (pace_globals.v)
#endif



#endif	/* PHP_PACE_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
