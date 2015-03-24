meta:
	ADDON_NAME = ofxMMAL
	ADDON_DESCRIPTION = MMAL shit
	ADDON_AUTHOR = Jason Van Cleave
	ADDON_TAGS = "raspberry pi, camera"
	ADDON_URL = https://github.com/jvcleave/ofxMMAL

common:
	# dependencies with other addons, a list of them separated by spaces 
	# or use += in several lines
	# ADDON_DEPENDENCIES =
		
	# include search paths, this will be usually parsed from the file system
	# but if the addon or addon libraries need special search paths they can be
	# specified here separated by spaces or one per line using +=
    #ADDON_INCLUDES =
		
	# any special flag that should be passed to the compiler when using this
	# addon
	ADDON_CFLAGS = -I/opt/vc/include/interface 
    ADDON_LDFLAGS = -lmmal -lmmal_util -lmmal_core 

