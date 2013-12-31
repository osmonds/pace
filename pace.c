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
  | Author:Osmond Sun      <osmond.sun@gmail.com>                        |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_variables.h"
#include "php_ini.h"
#include "zend.h"
#include "zend_extensions.h"
#include "ext/standard/info.h"
#include "php_pace.h"

/* If you declare any globals in php_pace.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(pace)
*/

/* True global resources - no need for thread safety here */
//static int le_pace;

static PHP_GINIT_FUNCTION(pace);
ZEND_DECLARE_MODULE_GLOBALS(pace);

#if PHP_VERSION_ID >= 50300
# define APPLY_TSRMLS_CC TSRMLS_CC
# define APPLY_TSRMLS_DC TSRMLS_DC
#else
# define APPLY_TSRMLS_CC
# define APPLY_TSRMLS_DC
#endif


#if PHP_VERSION_ID >= 50500
static void (*old_zend_execute_ex)(zend_execute_data * execute_data TSRMLS_DC);
static void pace_zend_execute_ex(zend_execute_data * execute_data TSRMLS_DC);
#else
static void (*old_zend_execute)(zend_op_array * op_array TSRMLS_DC);
static void pace_zend_execute(zend_op_array * op_array TSRMLS_DC);
#endif

zend_op_array*(*old_zend_compile_file)(zend_file_handle * file_handle, int type TSRMLS_DC);
zend_op_array* pace_zend_compile_file(zend_file_handle * file_handle, int type TSRMLS_DC);

int zend_pace_initialised = 0;

/* {{{ pace_functions[]
 *
 * Every user visible function must have an entry in pace_functions[].
 */
const zend_function_entry pace_functions[] = {
    {NULL, NULL, NULL}
	//PHP_FE_END	/* Must be the last line in pace_functions[] */
};
/* }}} */

/* {{{ pace_module_entry
 */
zend_module_entry pace_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"pace",
	pace_functions,
	PHP_MINIT(pace),
	PHP_MSHUTDOWN(pace),
	PHP_RINIT(pace),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(pace),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(pace),
#if ZEND_MODULE_API_NO >= 20010901
	PACE_VERSION,
#endif
#if 1
    PHP_MODULE_GLOBALS(pace),
    PHP_GINIT(pace),
    NULL,
    NULL,
#endif
	STANDARD_MODULE_PROPERTIES_EX
};
/* }}} */

#ifdef COMPILE_DL_PACE
ZEND_GET_MODULE(pace)
#endif

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
 */
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("pace.global_value",      "42", PHP_INI_SYSTEM, OnUpdateString, global_value, zend_pace_globals, pace_globals)
    STD_PHP_INI_ENTRY("pace.global_string", "foobar", PHP_INI_SYSTEM, OnUpdateString, global_string, zend_pace_globals, pace_globals)
PHP_INI_END()
/* }}} */

/* {{{ php_pace_init_globals
 */
static void php_pace_init_globals(zend_pace_globals *pace_globals)
{
	pace_globals->global_value = NULL;
	pace_globals->global_string = NULL;
}
/* }}} */

/*
 * Called to initialize a modules' globals before PHP_MINIT_FUNCTION.
 */
static PHP_GINIT_FUNCTION(pace)
{

}

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(pace)
{
	/* If you have INI entries, uncomment these lines 
	*/
	REGISTER_INI_ENTRIES();
   
    if (PACE_G(global_value) && strlen(PACE_G(global_value)) > 0)
        PACE_G(global_value) = PACE_G(global_value);

	if (zend_pace_initialised == 0)
    {
		zend_error(E_ERROR, "pace extension MUST be loaded as a Zend extension!");
		return FAILURE;
	} 
    
    //zend_error(E_WARNING, "init,MINIT,here:%s", __func__);
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(pace)
{
	/* uncomment this line if you have INI entries
	*/
	UNREGISTER_INI_ENTRIES();
	
    return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(pace)
{
	//zend_error(E_WARNING, "init,rinit,%s", __func__);
	old_zend_compile_file = zend_compile_file;
	zend_compile_file = pace_zend_compile_file;

	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(pace)
{
    zend_compile_file = old_zend_compile_file;

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(pace)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "pace (PHP access control extension support", "enabled");
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	*/
	DISPLAY_INI_ENTRIES();
}
/* }}} */

int php_execute_check(zend_file_handle *file_handle, int type)
{
	int ret;
	//zend_error(E_WARNING, "!!!!!type[%d]", type);
	ret = php_execute_check_selinux(file_handle, type);
	//zend_error(E_WARNING, "check!!!!!:[%s]_end_:[%d]type[%d]", file_handle->filename, ret,type);
	return ret;
}

/*
 *  zend_compile_file handler
 */
zend_op_array * pace_zend_compile_file(zend_file_handle * file_handle, int type TSRMLS_DC)
{
	zend_op_array * compiled_op_array;
    //zend_error(E_WARNING, "%s,[%s]",__func__, file_handle->filename);
#if 1
	int ret;
	ret = php_execute_check(file_handle, type);
	if(ret){
		zend_error(E_ERROR,"permission denied");
	}
#endif
    compiled_op_array = old_zend_compile_file(file_handle, type TSRMLS_CC);
	return compiled_op_array;
}

/* Remove the following function when you have successfully modified config.m4
   so that your module can be compiled into PHP, it exists only for testing
   purposes. */

/* Every user-visible function in PHP should document itself in the source */
/* {{{ proto string confirm_pace_compiled(string arg)
   Return a string to confirm that the module is compiled in */
PHP_FUNCTION(confirm_pace_compiled)
{
	char *arg = NULL;
	int arg_len, len;
	char *strg;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &arg, &arg_len) == FAILURE) {
		return;
	}

	len = spprintf(&strg, 0, "Congratulations! You have successfully modified ext/%.78s/config.m4. Module %.78s is now compiled into PHP.", "pace", arg);
	RETURN_STRINGL(strg, len, 0);
}
/* }}} */
/* The previous line is meant for vim and emacs, so it can correctly fold and 
   unfold functions in source code. See the corresponding marks just before 
   function definition, where the functions purpose is also documented. Please 
   follow this convention for the convenience of others editing your code.
*/

ZEND_DLEXPORT int pace_zend_startup(zend_extension *extension)
{
        zend_pace_initialised = 1;
     //   zend_error(E_WARNING, "OSMOND,startup");
        return zend_startup_module(&pace_module_entry);
}

ZEND_DLEXPORT void pace_zend_shutdown(zend_extension *extension)
{
        // Nothing
}


#if 1

/* This is a Zend  extension */
#ifndef ZEND_EXT_API
#define ZEND_EXT_API	ZEND_DLEXPORT
#endif
ZEND_EXTENSION();
ZEND_DLEXPORT zend_extension zend_extension_entry = {
       	PACE_NAME,
		PACE_VERSION,
		PACE_AUTHOR,
		PACE_URL_FAQ,
		PACE_COPYRIGHT_SHORT, 
        pace_zend_startup,      // startup_func_t
        pace_zend_shutdown,     // shutdown_func_t
        NULL,                                   // activate_func_t
        NULL,                                   // deactivate_func_t
        NULL,                                   // message_handler_func_t
        NULL,                                   // op_array_handler_func_t
        NULL,                                   // statement_handler_func_t
        NULL,                                   // fcall_begin_handler_func_t
        NULL,                                   // fcall_end_handler_func_t
        NULL,                                   // op_array_ctor_func_t
        NULL,                                   // op_array_dtor_func_t
        STANDARD_ZEND_EXTENSION_PROPERTIES
};
#endif
/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
