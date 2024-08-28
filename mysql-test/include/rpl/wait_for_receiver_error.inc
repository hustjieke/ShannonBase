# ==== Purpose ====
#
# Waits until the IO thread of the current connection has got an
# error, or until a timeout is reached. Also waits until the IO
# thread has completely stopped.
#
#
# ==== Usage ====
#
# --let $slave_io_errno= NUMBER [, NUMBER ...] [# comment]
# [--let $show_slave_io_error= 1]
# [--let $slave_io_error_is_nonfatal= 1]
# [--let $slave_io_error_replace= REGEX]
# [--let $rpl_channel_name= 'NAME']
# [--let $rpl_debug= 1]
# [--let $slave_timeout= NUMBER]
# --source include/rpl/wait_for_receiver_error.inc
#
# Parameters:
#   $slave_io_errno
#     The expected IO error numbers. This can be either a single
#     number, or a comma-separated list of numbers. Examples:
#       --let $slave_io_errno= 1040, 1053, 2002, 2003, 2006, 2013
#       --let $slave_io_errno= 1593 # ER_REPLICA_FATAL_ERROR
#     It is allowed to add a comment at the end of the line, and it
#     is strongly advised to put the symbolic name of the error in
#     this comment, like the example above.
#     (After BUG#41956 has been fixed, this will be required to be
#     symbolic names instead of numbers.)
#
#   $show_slave_io_error
#     If set, will print the error to the query log.
#
#   $slave_io_error_is_nonfatal
#     By default, we wait for the slave IO thread to stop completely
#     (i.e., until Replica_IO_State is empty).  If this variable is set,
#     then we don't wait. This is useful if the error is non-fatal
#     (e.g., temporary connection error) and does not cause the slave
#     IO thread to stop.
# 
#   $slave_io_error_replace
#     If set, one or more regex patterns for replacing variable
#     text in the error message. Syntax as --replace-regex
#
#   $rpl_channel_name
#     See include/rpl/wait_for_replica_status.inc.
#
#   $slave_timeout
#     See include/rpl/wait_for_replica_status.inc
# 
#   $rpl_debug
#     See include/rpl/init.inc


--let $include_filename= rpl/wait_for_receiver_error.inc [errno=$slave_io_errno]
if ($rpl_channel_name)
{
  --let $include_filename= rpl/wait_for_receiver_error.inc [errno=$slave_io_errno FOR CHANNEL $rpl_channel_name]
}
--source include/begin_include_file.inc


# Wait until an IO Error different from zero is found in result of the query
# 'SHOW REPLICA STATUS'
let $old_slave_param_comparison= $slave_param_comparison;
let $slave_param= Last_IO_Errno;
let $slave_param_comparison= !=;
let $slave_param_value= 0;
source include/rpl/wait_for_replica_status.inc;
let $slave_param_comparison= $old_slave_param_comparison;

# Get the IO error number from the 'SHOW REPLICA STATUS' query which returned error
let $_wait_for_slave_io_errno= $_show_slave_status_value;

if ($slave_io_errno == '')
{
  --echo !!!ERROR IN TEST: you must set \$slave_io_errno before you source
  --echo !!!rpl/wait_for_receiever_error.inc. The error we got this time was '$_wait_for_slave_io_errno',
  --echo !!!so you probably want to add the following line to your test case:
  --echo !!!  --let \$slave_io_errno= $_wait_for_slave_io_errno
  --vertical_results
  eval $slave_param_statement;
  --die !!!ERROR IN TEST: you must set \$slave_io_errno before sourcing rpl/wait_for_receiever_error.inc
}

# Strip away comments
--let $_slave_io_errno= `SELECT IF(LOCATE('#', '$slave_io_errno') != 0, SUBSTR('$slave_io_errno', 1, LOCATE('#', '$slave_io_errno') - 1), '$slave_io_errno')`
if ($rpl_debug)
{
  --echo # debug: slave_io_errno='$slave_io_errno'; stripped from comments='$_slave_io_errno' wait_for_slave_io_errno='$_wait_for_slave_io_errno'
}

if (`SELECT $_wait_for_slave_io_errno NOT IN ($_slave_io_errno)`)
{
  --echo **** Slave stopped with wrong error code: $_wait_for_slave_io_errno (expected $slave_io_errno) ****
  source include/rpl/debug/show_debug_info.inc;
  --echo **** Slave stopped with wrong error code: $_wait_for_slave_io_errno (expected $slave_io_errno) ****
  --die Slave stopped with wrong error code
}

if ($show_slave_io_error)
{
  --let $_wait_for_slave_io_error_old_status_items= $status_items
  --let $status_items= Last_IO_Error
  --let $slave_field_result_replace= $slave_io_error_replace
  --source include/rpl/deprecated/show_replica_status.inc
  --let $status_items= $_wait_for_slave_io_error_old_status_items
}

if (!$slave_io_error_is_nonfatal)
{
  --let $slave_param= Replica_IO_State
  --let $slave_param_value=
  --source include/rpl/wait_for_replica_status.inc
}
--let $slave_io_error_is_nonfatal= 0


--let $include_filename= rpl/wait_for_receiver_error.inc [errno=$slave_io_errno]
--source include/end_include_file.inc