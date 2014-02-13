pace
====

PHP access control extension support

This is a php's extension, it can add access control extension support.

Installation
============
1. phpize
2. ./configure
3. make
4. sudo make install
5. cd selinux_policy
6. make -f /usr/share/selinux/devel/Makefile
7. semodule -i pace.pp

Configuration
=============
Create a php's config file with the following line to /etc/php.d based on your system.
zend_extension=/usr/lib64/php/modules/


