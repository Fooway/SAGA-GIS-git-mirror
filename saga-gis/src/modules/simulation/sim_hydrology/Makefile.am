#
# $Id: Makefile.am 1277 2011-12-22 15:48:59Z reklov_w $
#
if DEBUG
DBGFLAGS = -g -DDEBUG
endif
if SAGA_UNICODE
UC_DEFS = -D_SAGA_UNICODE
endif
DEF_SAGA           = -D_SAGA_LINUX -D_TYPEDEF_BYTE -D_TYPEDEF_WORD
CXX_INCS           = -I$(top_srcdir)/src/saga_core
AM_CXXFLAGS        = -fPIC $(CXX_INCS) $(DEF_SAGA) $(UC_DEFS) $(DBGFLAGS) $(GOMPFLAGS)
AM_LDFLAGS         = -fPIC -shared -avoid-version
pkglib_LTLIBRARIES = libsim_hydrology.la
libsim_hydrology_la_SOURCES =\
DVWK_SoilMoisture.cpp\
diffuse_pollution_risk.cpp\
diffusion_gradient_concentration.cpp\
idw.cpp\
KinWav_D8.cpp\
MLB_Interface.cpp\
topmodel.cpp\
topmodel_values.cpp\
WaterRetentionCapacity.cpp\
diffuse_pollution_risk.h\
diffusion_gradient_concentration.h\
DVWK_SoilMoisture.h\
idw.h\
KinWav_D8.h\
MLB_Interface.h\
topmodel.h\
topmodel_values.h\
WaterRetentionCapacity.h

libsim_hydrology_la_LIBADD = $(top_srcdir)/src/saga_core/saga_api/libsaga_api.la

