ACLOCAL_AMFLAGS = -I m4
PROJ=emc
SRCS=emc.cc emc_avplayer.cc

emc_LIBADD = -L/home/ccarvalho/.dev/lib -lelementary -lm -leet -lecore_evas -lecore_file -lecore_input -lethumb_client -lemotion -lecore_imf -lecore_con -leldbus -lefreet -lefreet_mime -lefreet_trash -leio -leo -leina -lpthread -levas -lecore -ledje  
emc_CFLAGS = -pthread -D_REENTRANT -I/home/ccarvalho/.dev/include/elementary-1 -I/home/ccarvalho/.dev/include/efl-1 -I/home/ccarvalho/.dev/include/ecore-x-1 -I/home/ccarvalho/.dev/include/eina-1 -I/home/ccarvalho/.dev/include/eina-1/eina -I/home/ccarvalho/.dev/include/eet-1 -I/home/ccarvalho/.dev/include/evas-1 -I/home/ccarvalho/.dev/include/ecore-1 -I/home/ccarvalho/.dev/include/ecore-evas-1 -I/home/ccarvalho/.dev/include/ecore-file-1 -I/home/ccarvalho/.dev/include/ecore-input-1 -I/home/ccarvalho/.dev/include/edje-1 -I/home/ccarvalho/.dev/include/eo-1 -I/home/ccarvalho/.dev/include/ethumb-client-1 -I/home/ccarvalho/.dev/include/emotion-1 -I/home/ccarvalho/.dev/include/ecore-imf-1 -I/home/ccarvalho/.dev/include/ecore-con-1 -I/home/ccarvalho/.dev/include/eio-1 -I/home/ccarvalho/.dev/include/eldbus-1 -I/home/ccarvalho/.dev/include/efreet-1 -I/home/ccarvalho/.dev/include/eo-cxx-1 -I/home/ccarvalho/.dev/include/eina-cxx-1 -I/home/ccarvalho/.dev/include/eina-cxx-1/eina-cxx -I/home/ccarvalho/.dev/include/evas-cxx-1 -I/home/ccarvalho/.dev/include/evas-cxx-1/canvas -I/home/ccarvalho/.dev/include/evas-1/canvas -I/home/ccarvalho/.dev/include/ecore-input-evas-1 -I/home/ccarvalho/.dev/include/ecore-audio-1 -I/home/ccarvalho/.dev/include/ephysics-1 -I/home/ccarvalho/.dev/include/embryo-1 -I/home/ccarvalho/.dev/include/ecore-imf-evas-1 -I/home/ccarvalho/.dev/include/ethumb-1 -I/home/ccarvalho/.dev/include/eeze-1 -I/home/ccarvalho/.dev/include/edje-cxx-1 -I/home/ccarvalho/.dev/include/ecore-cxx-1 -I/home/ccarvalho/.dev/include/eolian-cxx-1 -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr//include/luajit-2.0 -I/usr/include/libpng12 -I/usr/include/fribidi -I/usr/include/freetype2 -I/usr/include/bullet -I/usr/include/dbus-1.0 -I/usr/lib/x86_64-linux-gnu/dbus-1.0/include -I/usr/include/libmount -I/usr/include/blkid -I/usr/include/uuid  
CC = g++

DEFS = -DEFL_BETA_API_SUPPORT \
-DELM_INTERNAL_API_ARGESFSDFEFC \
-DELM_INTERFACE_ATSPI_ACCESSIBLE_PROTECTED \
-DELM_INTERFACE_ATSPI_COMPONENT_PROTECTED \
-DELM_INTERFACE_ATSPI_ACTION_PROTECTED \
-DELM_INTERFACE_ATSPI_VALUE_PROTECTED \
-DELM_INTERFACE_ATSPI_EDITABLE_TEXT_PROTECTED \
-DELM_INTERFACE_ATSPI_TEXT_PROTECTED \
-DELM_INTERFACE_ATSPI_SELECTION_PROTECTED \
-DELM_INTERFACE_ATSPI_IMAGE_PROTECTED \
-DELM_INTERFACE_ATSPI_WIDGET_ACTION_PROTECTED

DEFS += -DEMC_DISABLE_EO_CALLBACK

all:
	$(CC) $(SRCS) -std=c++11 ${emc_LIBADD}  ${emc_CFLAGS} ${DEFS} -o $(PROJ)

clean:
	rm -f emc *.o
