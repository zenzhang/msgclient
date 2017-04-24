.PHONY: target
target: | mkdirs

MAKE_DIRECTORYS		:= $(TARGET_DIRECTORY)

include $(ROOT_MAKE_DIRECTORY)/mkdirs.mk

CONFIG_build_version	:= no

include $(PROJ_MAKE_DIRECTORY)/publish.mk

PUBLISH_FILE		:= $(PRIVATE_DIRECTORY)/$(TARGET_FILE_PRIVATE)

$(TARGET_DIRECTORY)/$(TARGET_FILE_PRIVATE): $(PUBLISH_FILE) | mkdirs
	$(LN) $(PUBLISH_FILE) $(TARGET_DIRECTORY)/$(TARGET_FILE_PRIVATE)
	
target: $(TARGET_DIRECTORY)/$(TARGET_FILE_PRIVATE)