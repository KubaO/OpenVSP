CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(UTIL_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE PATH "Path to util headers")

INCLUDE_DIRECTORIES(
    ${NANOFLANN_INCLUDE_DIR}
    ${XMLVSP_INCLUDE_DIR}
    ${UTIL_INCLUDE_DIR}
    ${CPPTEST_INCLUDE_DIRS}
    ${EIGEN3_INCLUDE_DIR}
    ${CodeEli_INCLUDE_DIRS}
    ${STEPCODE_INCLUDE_DIR}
    )

ADD_LIBRARY(util
BndBox.cpp
DrawObj.cpp
ExitStatus.cpp
FileUtil.cpp
Matrix.cpp
MessageMgr.cpp
PntNodeMerge.cpp
Quat.cpp
STEPutil.cpp
StlHelper.cpp
StringUtil.cpp
SuperEllipse.cpp
Util.cpp
UtilTestSuite.cpp
Vec2d.cpp
Vec3d.cpp
VspCurve.cpp
VspObj.cpp
VspSurf.cpp
BndBox.h
Combination.h
Defines.h
DrawObj.h
ExitStatus.h
FileUtil.h
GuiDeviceEnums.h
Matrix.h
MessageMgr.h
PntNodeMerge.h
Quat.h
StlHelper.h
STEPutil.h
StreamUtil.h
StringUtil.h
SuperEllipse.h
Util.h
UtilTestSuite.h
UsingCpp11.h
Vec2d.h
Vec3d.h
VspCurve.h
VspObj.h
VspSurf.h
WriteMatlab.h
)

QT5_USE_MODULES(util Core)

ADD_DEPENDENCIES( util
STEPCODE
)
