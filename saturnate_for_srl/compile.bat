:; export PATH="$PATH:$(realpath ../../Compiler/linux/sh2eb-elf/bin)"; "../../tools/scripts/make.sh" $1; exit;
@ECHO Off
"../../tools/scripts/make.bat" %1
