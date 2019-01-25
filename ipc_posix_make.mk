define $(_flavor_)_$(_feat_)_MAKE
$(_flavor_)_$(_feat_)_inc=\
ipc_posix.h \
posix_thread.h \
posix_mbx.h \
posix_mux.h \
posix_sem.h \
posix_cv.h \
posix_tmr.h \

$(_flavor_)_$(_feat_)_lib_objs=\
ipc_posix \
posix_thread \
posix_mbx \
posix_mux \
posix_sem \
posix_cv \
posix_tmr \

$(_flavor_)_$(_feat_)_lib=ipc_posix

endef

ifndef $($(_flavor_)_build)_PROJ_LIBS
$($(_flavor_)_build)_PROJ_LIBS+=\
	-lpthread \
	#-lrt

endif

-include $(PROJ_MAK_DIR)/epilog.mk
