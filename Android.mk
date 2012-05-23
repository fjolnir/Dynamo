LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

DEPS_PATH = $(LOCAL_PATH)/Dependencies
LOCAL_CFLAGS := -I$(DEPS_PATH) -std=gnu99 -I$(DEPS_PATH)/png -I$(DEPS_PATH)/mxml -I$(DEPS_PATH)/Chipmunk/include -I$(DEPS_PATH)/Chipmunk/include/chipmunk  -g -DDYNAMO_DEBUG -DCP_USE_DOUBLES=0 -DANDROID_APP_IDENTIFIER=\"$(APP_IDENTIFIER)\"
LOCAL_CXXFLAGS := -std=gnu++98
LOCAL_ARM_MODE := arm
TARGET_PLATFORM := android-9
TARGET_ABI := android-9-armeabi

LOCAL_MODULE    := dynamo
LOCAL_SRC_FILES :=\
Source/array.c \
Source/background.c \
Source/drawutils.c \
Source/gametimer.c \
Source/input.c \
Source/linkedlist.c \
Source/object.c \
Source/png_loader.c \
Source/renderer.c \
Source/scene.c \
Source/shader.c \
Source/sprite.c \
Source/texture.c \
Source/texture_atlas.c \
Source/tmx_map.c \
Source/util.c \
Source/sound_android.c \
Source/dictionary.c \
Source/json.c \
Source/primitive_types.c \
Source/world.c \
Dependencies/GLMath/GLMath.c \
Dependencies/GLMath/GLMathUtilities.c \
Dependencies/mxml/mxml-attr.c \
Dependencies/mxml/mxml-entity.c \
Dependencies/mxml/mxml-file.c \
Dependencies/mxml/mxml-get.c \
Dependencies/mxml/mxml-index.c \
Dependencies/mxml/mxml-node.c \
Dependencies/mxml/mxml-private.c \
Dependencies/mxml/mxml-search.c \
Dependencies/mxml/mxml-set.c \
Dependencies/mxml/mxml-string.c \
Dependencies/png/png.c \
Dependencies/png/pngerror.c \
Dependencies/png/pngget.c \
Dependencies/png/pngmem.c \
Dependencies/png/pngpread.c \
Dependencies/png/pngread.c \
Dependencies/png/pngrio.c \
Dependencies/png/pngrtran.c \
Dependencies/png/pngrutil.c \
Dependencies/png/pngset.c \
Dependencies/png/pngtrans.c \
Dependencies/png/pngwio.c \
Dependencies/png/pngwrite.c \
Dependencies/png/pngwtran.c \
Dependencies/png/pngwutil.c \
Dependencies/yajl/yajl.c \
Dependencies/yajl/yajl_alloc.c \
Dependencies/yajl/yajl_buf.c \
Dependencies/yajl/yajl_encode.c \
Dependencies/yajl/yajl_gen.c \
Dependencies/yajl/yajl_lex.c \
Dependencies/yajl/yajl_parser.c \
Dependencies/yajl/yajl_tree.c \
Dependencies/yajl/yajl_version.c \
Dependencies/Chipmunk/src/chipmunk.c \
Dependencies/Chipmunk/src/constraints/cpConstraint.c \
Dependencies/Chipmunk/src/constraints/cpDampedRotarySpring.c \
Dependencies/Chipmunk/src/constraints/cpDampedSpring.c \
Dependencies/Chipmunk/src/constraints/cpGearJoint.c \
Dependencies/Chipmunk/src/constraints/cpGrooveJoint.c \
Dependencies/Chipmunk/src/constraints/cpPinJoint.c \
Dependencies/Chipmunk/src/constraints/cpPivotJoint.c \
Dependencies/Chipmunk/src/constraints/cpRatchetJoint.c \
Dependencies/Chipmunk/src/constraints/cpRotaryLimitJoint.c \
Dependencies/Chipmunk/src/constraints/cpSimpleMotor.c \
Dependencies/Chipmunk/src/constraints/cpSlideJoint.c \
Dependencies/Chipmunk/src/cpArbiter.c \
Dependencies/Chipmunk/src/cpArray.c \
Dependencies/Chipmunk/src/cpBB.c \
Dependencies/Chipmunk/src/cpBBTree.c \
Dependencies/Chipmunk/src/cpBody.c \
Dependencies/Chipmunk/src/cpCollision.c \
Dependencies/Chipmunk/src/cpHashSet.c \
Dependencies/Chipmunk/src/cpPolyShape.c \
Dependencies/Chipmunk/src/cpShape.c \
Dependencies/Chipmunk/src/cpSpace.c \
Dependencies/Chipmunk/src/cpSpaceComponent.c \
Dependencies/Chipmunk/src/cpSpaceHash.c \
Dependencies/Chipmunk/src/cpSpaceQuery.c \
Dependencies/Chipmunk/src/cpSpaceStep.c \
Dependencies/Chipmunk/src/cpSpatialIndex.c \
Dependencies/Chipmunk/src/cpSweep1D.c \
Dependencies/Chipmunk/src/cpVect.c

LIBS+=bps screen EGL GLESv2 freetype
LOCAL_LDLIBS := -lz -llog -ldl -lGLESv2 -lOpenSLES

include $(BUILD_SHARED_LIBRARY)

