FILE_SYSTEM_NAME = FileSystem
INIT_FILE_NAME = ./src/init
SHELL_FILE_NAME = ./src/server

LINK_OPTIONS = ./src/Filesystem/filesystem.c ./src/Types/inode.c ./src/Types/superblock.c \
			   ./src/Types/mapping.c ./src/Filesystem/commands.c
run :
	touch $(FILE_SYSTEM_NAME)
	gcc -o $(INIT_FILE_NAME) $(INIT_FILE_NAME).c $(LINK_OPTIONS)
	$(INIT_FILE_NAME)
	$(RM) $(INIT_FILE_NAME)
	$(RM) $(INIT_FILE_NAME).o

	gcc -o server $(SHELL_FILE_NAME).c $(LINK_OPTIONS)
	gcc -o client ./src/client.c