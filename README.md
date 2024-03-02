# OpenFolderAndSelectItem
Tiny EXE to open folder and select a file/folder in Windows Explorer.

### Supported OS

* Windows XP to Windows 11

### Tiny overhead
* 3.0KB EXE for 32-bit Windows (i386)
* 4.5KB EXE for 64-bit Windows (AMD64)
* No additional DLL/redistribute/runtime needed

### Easy to use

Open "C:\Windows" and select "System32" folder.
```batch
OpenFolderAndSelect.exe "C:\Windows\System32"
```

Open "C:\Windows\System32" and select "cmd.exe".
```batch
OpenFolderAndSelect.exe "C:\Windows\System32\cmd.exe"
```

Open "C:\Windows\System32" and select all items where name ended with ".exe".
```batch
OpenFolderAndSelect.exe "C:\Windows\System32\*.exe"
```

Open "C:\" and select two folders ("Program Files" and "Windows").
```batch
OpenFolderAndSelect.exe C:\ "Program Files" Windows
```

# Why?

Why do I go through the trouble to create this? Why should we use this?

Did you ever troubled by multiple instances of "explorer.exe" spawned from nowhere, thinking your PC might be compromised or what not?
![multiple-explorer-processes.png](https://raw.githubusercontent.com/Raymai97/OpenFolderAndSelectItem/trunk/github-assets/multiple-explorer-processes.png)

What if I tell you, it could be caused by the command usage below?
```
explorer /select,<file path here>
```

A simple web search can tell that `explorer /select,` is one of the most popular way to "open containing folder" of a file or folder, and it was rightfully so, because it worked so well since the era of Windows 95.

However, starting from Windows 7, it will spawn a new "explorer.exe" process for every invocation, even when "Launch folder windows in a separate process" in the "Folder Options" is UNTICKED.

![explorer-select-is-causing-it.gif](https://raw.githubusercontent.com/Raymai97/OpenFolderAndSelectItem/trunk/github-assets/explorer-select-is-causing-it.gif)

I do not believe I'm the only one who really hated this, so I created this program and hope that in future, more and more programs will move away from `explorer /select`.
