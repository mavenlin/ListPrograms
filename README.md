ListPrograms
============

C++ Code to Show all Installed Programs on Windows

There are many codes to list all programs installed on windows.

Most frequently seen is:

1. Listing all keys in HKEY_LOCAL_MACHINE\Software\Microsoft\Windows\CurrentVersion\Uninstall.

2. Win32_Product WMI (Windows Management Instrumentation) class.

3. MSI API.

However, none of these is clean way to obtain all the softwares. By all, I mean all the software listed in Control Panel > Programs > Programs and Features > Uninstall or change a Program.

This post on stackoverflow http://stackoverflow.com/questions/802499/how-can-i-enumerate-list-all-installed-applications-in-windows-xp/9757013#comment22371865_9757013

One of the answers showed some code on some VB forum, which works out and is able to show the same programs as listed in Control Panel.
Though I don't understand why that answer has only one vote up (I did it). But this code is the best I ever see.

However, it is written in VB, not so popular language.

I ported here using C++.

Things changed:

1. Using the standard method to access Wow6432Node.
2. More Informations are collected, InstallLocation, Icon and Architecture of the binary (X86 or X64).
3. In future will include the code to extract Icons.
