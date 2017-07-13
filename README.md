ExecInExplorer
==============

ExecInExplorer - Execute a command asynchronously in the context of explorer

This is a fork of
[Microsoft's ExecInExplorer sample](https://github.com/Microsoft/Windows-classic-samples/tree/master/Samples/Win7Samples/winui/shell/appplatform/ExecInExplorer).
I modified it to execute the program arguments as a command line and optionally
use the current directory.

### Usage

Usage: `ExecInExplorer [--use-current-directory[=dir]] <prog> [[arg]...]`  
Execute a command asynchronously in the context of explorer.

ExecInExplorer is useful if you are running as administrator and you want to
execute a program in the context of the user's explorer (eg it will de-elevate
the program if explorer isn't running as admin).

Use option `--use-current-directory` to tell explorer to use this program's
current directory or optionally 'dir'. Due to the command line being parsed as
raw data in order to preserve it for explorer, this option is parsed raw
(escapes are not acknowledged), must not contain quotes but can be quoted and
if quoted ends immediately at the next quote no matter what. For example:

~~~
Yes:  ExecInExplorer "--use-current-directory=C:\foo bar\baz" notepad foo
No:   ExecInExplorer --use-current-directory="C:\foo bar\baz" notepad foo
No:   ExecInExplorer "--use-current-directory=\"C:\foo bar\baz\"" notepad foo
~~~

If the specified directory is invalid or inaccessible the behavior is
undocumented. Empirical testing shows if the directory does not exist then
explorer will use the root directory of the drive if the drive exists or the
system root (typically C:\Windows) if it doesn't. If you do not use option
`--use-current-directory` then explorer will use its current directory
(typically C:\Windows\System32).

The exit code returned by ExecInExplorer is 0 if the command line to be
executed was *dispatched* successfully or otherwise an HRESULT indicating why
it wasn't. There is no other interaction.

Other
-----

### Resources

Raymond Chen's
[blog post](https://blogs.msdn.microsoft.com/oldnewthing/20131118-00/?p=2643).

ExecInExplorer works by calling function
[IShellDispatch2.ShellExecute](https://msdn.microsoft.com/en-us/library/windows/desktop/bb774148.aspx).

[Microsoft's RunAsDesktopUser sample](https://blogs.msdn.microsoft.com/aaron_margosis/2009/06/06/faq-how-do-i-start-a-program-as-the-desktop-user-from-an-elevated-app/),
which is similar to ExecInExplorer. In
[my RunAsDesktopUser fork](https://github.com/jay/RunAsDesktopUser)
I made changes similar to what I made in ExecInExplorer, and also a change
allowing for synchronous execution of the command line (via
--wait-for-exit-code).

### Incorrect exit code when run from Windows Command Prompt?

ExecInExplorer uses the Windows subsystem not the console subsystem, which
means that when you execute it from the Windows command interpreter the
interpreter runs it asynchronously and therefore does not update %ERRORLEVEL%
with the exit code. However if you execute it from a batch file the interpreter
will run it synchronously and once it terminates its exit code will be put in
%ERRORLEVEL%. None of this has any bearing on how ExecInExplorer executes the
command line passed to it, which is always asynchronous.

### License

[MIT license](https://github.com/jay/ExecInExplorer/blob/master/LICENSE)

### Source

The source can be found on
[GitHub](https://github.com/jay/ExecInExplorer).
Since you're reading this maybe you're already there?

### Send me any questions you have

Jay Satiro `<raysatiro$at$yahoo{}com>` and put ExecInExplorer in the subject.
