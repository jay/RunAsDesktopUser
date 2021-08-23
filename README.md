RunAsDesktopUser
================

RunAsDesktopUser - Execute a command in the context of the desktop user

This is a fork of
[Microsoft's RunAsDesktopUser sample](https://blogs.msdn.microsoft.com/aaron_margosis/2009/06/06/faq-how-do-i-start-a-program-as-the-desktop-user-from-an-elevated-app/).
I modified it to execute the program arguments as a command line and optionally
use the current directory, wait for process termination and fall back to the
current user if not an administrator. It works on Windows Vista or greater.

### Usage
Usage: `RunAsDesktopUser [--use-current-directory[=dir]] 
[--fallback-current-user] [--wait-for-exit-code] <prog> [[arg]...]`  
Execute a command in the context of the desktop user.

RunAsDesktopUser is useful if you are running as administrator and you want to
execute a program in the context of the desktop user (eg it will de-elevate the
program if the desktop user's shell isn't running as admin).

Use option `--fallback-current-user` to fall back to executing in the context
of the current user running this program if that user is not running the
program with administrator privileges. If you do not use this option and are
not running this program as an administrator then this program will have no
effect.

Use option `--use-current-directory` to use this program's current directory or
optionally 'dir'. Due to the command line being parsed as raw data in order to
preserve it for CreateProcess, this option is parsed raw (escapes are not
acknowledged), must not contain quotes but can be quoted and if quoted ends
immediately at the next quote no matter what. For example:

~~~
Yes:  RunAsDesktopUser "--use-current-directory=C:\foo bar\baz" notepad foo
No:   RunAsDesktopUser --use-current-directory="C:\foo bar\baz" notepad foo
No:   RunAsDesktopUser "--use-current-directory=\"C:\foo bar\baz\"" notepad foo
~~~

If the specified directory is invalid or inaccessible the behavior is
undocumented. Empirical testing shows process creation will fail. If you do not
use option `--use-current-directory` then the behavior varies; it is documented
that both CreateProcessWithTokenW (admin) and CreateProcessW (nonadmin) will
use RunAsDesktopUser's current directory, however empirical testing shows the
former will use the system directory (typically C:\Windows\System32).

Use option `--wait-for-exit-code` to wait for the exit code of the 'prog'
process, and have this program return that exit code. If you do not use this
option then the exit code is 0 if prog was started. Whether this option is used
or not if an error occurs in RunAsDesktopUser the exit code is -1. There is no
other interaction.

Other
-----

### Resources

RunAsDesktopUser works by calling function
[CreateProcessWithTokenW](https://msdn.microsoft.com/en-us/library/ms682434.aspx)
if the program has admin privileges or
[CreateProcessW](https://msdn.microsoft.com/en-us/library/windows/desktop/ms682425.aspx)
if the program doesn't have admin privileges and option --fallback-current-user
was used.

[Microsoft's ExecInExplorer sample](https://github.com/Microsoft/Windows-classic-samples/tree/master/Samples/Win7Samples/winui/shell/appplatform/ExecInExplorer),
which is similar to RunAsDesktopUser. In
[my ExecInExplorer fork](https://github.com/jay/ExecInExplorer)
I made changes similar to what I made in RunAsDesktopUser, however due to
limitations of the method there is no synchronous execution of the command
line.

### Using explorer.exe for the same purpose

If explorer.exe is running it's possible to execute a program in that context
by using the full path to the program as the first argument, for example
`explorer.exe "C:\foo bar\baz"`. Program arguments are not accepted.
Unfortunately this feature is undocumented, has been called a bug and may
[disappear one day](http://mdb-blog.blogspot.com/2013/01/nsis-lunch-program-as-user-from-uac.html?showComment=1388694317801#c939517856791332836).

### Incorrect exit code when run from Windows Command Prompt?

RunAsDesktopUser uses the Windows subsystem not the console subsystem, which
means that when you execute it from the Windows command interpreter the
interpreter runs it asynchronously and therefore does not update %ERRORLEVEL%
with the exit code. However if you execute it from a batch file the interpreter
will run it synchronously and once it terminates its exit code will be put in
%ERRORLEVEL%. None of this has any bearing on how RunAsDesktopUser executes the
command line passed to it.

### License

[MIT license](https://github.com/jay/RunAsDesktopUser/blob/master/LICENSE)

### Source

The source can be found on
[GitHub](https://github.com/jay/RunAsDesktopUser).
Since you're reading this maybe you're already there?

### Send me any questions you have

Jay Satiro `<raysatiro$at$yahoo{}com>` and put RunAsDesktopUser in the subject.
