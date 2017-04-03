
INCLUDEPATH += $$PWD/ffmpeg/include

LIBS += -L$$PWD/ffmpeg/lib \
         -lavcodec \
         -lavdevice \
         -lavfilter \
         -lavformat \
         -lavresample \
         -lavutil \
         -lswresample \
         -lswscale


