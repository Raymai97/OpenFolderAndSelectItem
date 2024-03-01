cl -MD -O1 -GS- ^
	main.c ^
	-link ^
	kernel32.lib ^
	user32.lib ^
	ole32.lib ^
	-out:OpenFolderAndSelect.exe ^
	-entry:RawMain ^
	-fixed ^
	-subsystem:windows
