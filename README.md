
Secure Core Utilities
=====================

Secure Core Utilities is a re-implmentation of some coreutils programs
which are intended to be used with sudo in an attempt to provide secure
access to files without allowing unrestricted read access to any file.


Contents
--------

   1. Disclaimer
   2. Maintainers
   3. Background
   4. Utilities
   5. Source Code
   6. Package Maintence Notes


Disclaimer
==========

This software is provided by the copyright holders and contributors "as
is" and any express or implied warranties, including, but not limited to,
the implied warranties of merchantability and fitness for a particular
purpose are disclaimed. In no event shall David M. Syzdek be liable for
any direct, indirect, incidental, special, exemplary, or consequential
damages (including, but not limited to, procurement of substitute goods or
services; loss of use, data, or profits; or business interruption) however
caused and on any theory of liability, whether in contract, strict
liability, or tort (including negligence or otherwise) arising in any way
out of the use of this software, even if advised of the possibility of
such damage.


Maintainers
===========

   * David M. Syzdek <david@syzdek.net>


Background
==========

In some circumstances it may be useful to provide read access to files
within a particular directory, for example granting support staff the
ability to view log files.  This can be accomplished using sudo, however
sudo's use of glob() instead of regexec() forces the administrator to
enumerate each individual allowed file as a sudo command. Otherwise using
a wild card in the sudoers file may allow users to read additional files.

Consider the following configuration excerpt from a sudoers file:

      %support ALL=(ALL) /usr/bin/cat /var/log/*

The above configuration would allow a member of the support group to read
the /etc/shadow file using either of the following commands:

      $ sudo cat /var/log/ /etc/shadow

      $ sudo cat /var/log/../../etc/shadow

Secure Core Utilities attempts to mitigate this by placing the restrictions
on file paths.  If a file path violates one of the restrictions, the utility
will fail with an error. The following file path restrictions are enforced:
	
   * The period (‘.’) character and/or the forward slash (‘/‘) character are
     considered special characters. A special character may not be adjacent
     to another special character within the file path. The following table
     demonstrates valid and invalid paths:

           /var/log/messages           (valid)
           /var/log/file.txt           (valid)
           /var/log/file..txt          (invalid)
           /var/log//file.txt          (invalid)
           /var/log/../../etc/shadow   (invalid)
           /var/log/.someHiddenFile    (invalid)
           /var/log./file.txt          (invalid)

   * All file paths must be anchored with a leading slash. The following
     table demonstrates valid and invalid paths:

            /var/log/messages           (valid)
            ./var/log/message           (invalid)
            var/log/messages            (invalid)

   * Only one file may be specified on the command line unlike cat, grep,
     tail, head, etc which accept multiple input files on the command line.

   * No part of the the file path may be a symbolic link.

An administrator is able to use secure core utilities without changing
user's behavior by modifing the secure_path within the sudoers file and
creating a symlink.

Create the symlink for the secure core utilities:

      mkdir -p /usr/libexec/securecoreutils
      ln -s /usr/bin/securecoreutils /usr/libexec/securecoreutils/cat

The following is an example sudoers file:

      Defaults env_reset, \
         secure_path=/usr/libexec/securecoreutils:/usr/local/bin:/usr/bin:/bin:/usr/local/sbin:/usr/sbin:/sbin

      %support ALL=(ALL) /usr/libexec/securecoreutils/cat /var/log/*


Utilities
=========

   * bzcat          - Uncompresses file and write to standard out.
   * cat            - Writes contents of file to standard out.
   * gzcat          - Uncompresses file and write to standard out.
   * pathcheck      - Validates path using internal checks.
   * rm             - Removes a file.
   * rmdir          - Removes a directory.
   * tail           - Writes contents of file to standard out.
   * touch          - Updates access and modify timestamps of file.
   * zcat           - Uncompresses file and write to standard out (supports .bz2, .gz, .xz, .Z).
   * xzcat          - Uncompresses file and write to standard out.


Source Code
===========

The source code for this project is maintained using git
(http://git-scm.com).  The following contains information to checkout the
source code from the git repository.

Browse Source:

   * https://github.com/bindle/securecoreutils

Git URLs:

   * git://github.com/bindle/securecoreutils.git
   * https://github.com/bindle/bindleadmin.git
   * git@github.com:bindle/bindleadmin.git

Downloading Source:

      $ git clone git://github.com/bindle/securecoreutils.git

Preparing Source:

      $ cd securecoreutils
      $ ./autogen.sh

Git Branches:

   * master - Current release of packages.
   * next   - changes staged for next release
   * pu     - proposed updates for next release
   * xx/yy+ - branch for testing new changes before merging to 'pu' branch


Package Maintence Notes
=======================

This is a collection of notes for developers to use when maintaining this
package.

New Release Checklist:

   * Switch to 'master' branch in Git repository.
   * Update version in configure.ac.
   * Update date and version in ChangeLog.
   * Commit configure.ac and ChangeLog changes to repository.
   * Create tag in git repository:

           $ git tag -s v${MAJOR}.${MINOR}

   * Push repository to publishing server:

           $ git push --tags origin master:master next:next pu:pu

Creating Source Distribution Archives:

      $ ./configure
      $ make update
      $ make distcheck
      $ make dist-bzip2

