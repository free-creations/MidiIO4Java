#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux-x86
CND_DLIB_EXT=so
CND_CONF=Debug
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/jackNative.o \
	${OBJECTDIR}/windowsNative.o

# Test Directory
TESTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}/tests

# Test Files
TESTFILES= \
	${TESTDIR}/TestFiles/f2 \
	${TESTDIR}/TestFiles/f3 \
	${TESTDIR}/TestFiles/f1 \
	${TESTDIR}/TestFiles/f4

# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-lpthread `pkg-config --libs jack`  

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libnative.${CND_DLIB_EXT}

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libnative.${CND_DLIB_EXT}: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -shared -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libnative.${CND_DLIB_EXT} -fPIC ${OBJECTFILES} ${LDLIBSOPTIONS} 

${OBJECTDIR}/jackNative.o: nbproject/Makefile-${CND_CONF}.mk jackNative.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -Werror -D${WITH_JUNIT} -DWITH_JACK -I${JNI_INCLUDE_OS} -I${JNI_INCLUDE_BASE} `pkg-config --cflags jack` -std=c++11   -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/jackNative.o jackNative.cpp

${OBJECTDIR}/windowsNative.o: nbproject/Makefile-${CND_CONF}.mk windowsNative.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -Werror -D${WITH_JUNIT} -DWITH_JACK -I${JNI_INCLUDE_OS} -I${JNI_INCLUDE_BASE} `pkg-config --cflags jack` -std=c++11   -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/windowsNative.o windowsNative.cpp

# Subprojects
.build-subprojects:

# Build Test Targets
.build-tests-conf: .build-conf ${TESTFILES}
${TESTDIR}/TestFiles/f2: ${TESTDIR}/tests/portTest.o ${TESTDIR}/tests/portTestRunner.o ${OBJECTFILES:%.o=%_nomain.o}
	${MKDIR} -p ${TESTDIR}/TestFiles
	${LINK.cc}   -o ${TESTDIR}/TestFiles/f2 $^ ${LDLIBSOPTIONS} -lcppunit 

${TESTDIR}/TestFiles/f3: ${TESTDIR}/tests/portchainTest.o ${TESTDIR}/tests/portchainTestRunner.o ${OBJECTFILES:%.o=%_nomain.o}
	${MKDIR} -p ${TESTDIR}/TestFiles
	${LINK.cc}   -o ${TESTDIR}/TestFiles/f3 $^ ${LDLIBSOPTIONS} -lcppunit 

${TESTDIR}/TestFiles/f1: ${TESTDIR}/tests/processExceptionTest.o ${TESTDIR}/tests/processExceptionTestRunner.o ${OBJECTFILES:%.o=%_nomain.o}
	${MKDIR} -p ${TESTDIR}/TestFiles
	${LINK.cc}   -o ${TESTDIR}/TestFiles/f1 $^ ${LDLIBSOPTIONS} -lcppunit 

${TESTDIR}/TestFiles/f4: ${TESTDIR}/tests/ptrEnvelopeTest.o ${TESTDIR}/tests/ptrEnvelopeTestRunner.o ${OBJECTFILES:%.o=%_nomain.o}
	${MKDIR} -p ${TESTDIR}/TestFiles
	${LINK.cc}   -o ${TESTDIR}/TestFiles/f4 $^ ${LDLIBSOPTIONS} -lcppunit 


${TESTDIR}/tests/portTest.o: tests/portTest.cpp 
	${MKDIR} -p ${TESTDIR}/tests
	${RM} $@.d
	$(COMPILE.cc) -g -Werror -D${WITH_JUNIT} -DWITH_JACK -I. -I${JNI_INCLUDE_OS} -I${JNI_INCLUDE_BASE} `pkg-config --cflags jack` -std=c++11   -MMD -MP -MF $@.d -o ${TESTDIR}/tests/portTest.o tests/portTest.cpp


${TESTDIR}/tests/portTestRunner.o: tests/portTestRunner.cpp 
	${MKDIR} -p ${TESTDIR}/tests
	${RM} $@.d
	$(COMPILE.cc) -g -Werror -D${WITH_JUNIT} -DWITH_JACK -I. -I${JNI_INCLUDE_OS} -I${JNI_INCLUDE_BASE} `pkg-config --cflags jack` -std=c++11   -MMD -MP -MF $@.d -o ${TESTDIR}/tests/portTestRunner.o tests/portTestRunner.cpp


${TESTDIR}/tests/portchainTest.o: tests/portchainTest.cpp 
	${MKDIR} -p ${TESTDIR}/tests
	${RM} $@.d
	$(COMPILE.cc) -g -Werror -D${WITH_JUNIT} -DWITH_JACK -I. -I${JNI_INCLUDE_OS} -I${JNI_INCLUDE_BASE} `pkg-config --cflags jack` -std=c++11   -MMD -MP -MF $@.d -o ${TESTDIR}/tests/portchainTest.o tests/portchainTest.cpp


${TESTDIR}/tests/portchainTestRunner.o: tests/portchainTestRunner.cpp 
	${MKDIR} -p ${TESTDIR}/tests
	${RM} $@.d
	$(COMPILE.cc) -g -Werror -D${WITH_JUNIT} -DWITH_JACK -I. -I${JNI_INCLUDE_OS} -I${JNI_INCLUDE_BASE} `pkg-config --cflags jack` -std=c++11   -MMD -MP -MF $@.d -o ${TESTDIR}/tests/portchainTestRunner.o tests/portchainTestRunner.cpp


${TESTDIR}/tests/processExceptionTest.o: tests/processExceptionTest.cpp 
	${MKDIR} -p ${TESTDIR}/tests
	${RM} $@.d
	$(COMPILE.cc) -g -Werror -D${WITH_JUNIT} -DWITH_JACK -I. -I${JNI_INCLUDE_OS} -I${JNI_INCLUDE_BASE} `pkg-config --cflags jack` -std=c++11   -MMD -MP -MF $@.d -o ${TESTDIR}/tests/processExceptionTest.o tests/processExceptionTest.cpp


${TESTDIR}/tests/processExceptionTestRunner.o: tests/processExceptionTestRunner.cpp 
	${MKDIR} -p ${TESTDIR}/tests
	${RM} $@.d
	$(COMPILE.cc) -g -Werror -D${WITH_JUNIT} -DWITH_JACK -I. -I${JNI_INCLUDE_OS} -I${JNI_INCLUDE_BASE} `pkg-config --cflags jack` -std=c++11   -MMD -MP -MF $@.d -o ${TESTDIR}/tests/processExceptionTestRunner.o tests/processExceptionTestRunner.cpp


${TESTDIR}/tests/ptrEnvelopeTest.o: tests/ptrEnvelopeTest.cpp 
	${MKDIR} -p ${TESTDIR}/tests
	${RM} $@.d
	$(COMPILE.cc) -g -Werror -D${WITH_JUNIT} -DWITH_JACK -I. -I${JNI_INCLUDE_OS} -I${JNI_INCLUDE_BASE} `pkg-config --cflags jack` -std=c++11   -MMD -MP -MF $@.d -o ${TESTDIR}/tests/ptrEnvelopeTest.o tests/ptrEnvelopeTest.cpp


${TESTDIR}/tests/ptrEnvelopeTestRunner.o: tests/ptrEnvelopeTestRunner.cpp 
	${MKDIR} -p ${TESTDIR}/tests
	${RM} $@.d
	$(COMPILE.cc) -g -Werror -D${WITH_JUNIT} -DWITH_JACK -I. -I${JNI_INCLUDE_OS} -I${JNI_INCLUDE_BASE} `pkg-config --cflags jack` -std=c++11   -MMD -MP -MF $@.d -o ${TESTDIR}/tests/ptrEnvelopeTestRunner.o tests/ptrEnvelopeTestRunner.cpp


${OBJECTDIR}/jackNative_nomain.o: ${OBJECTDIR}/jackNative.o jackNative.cpp 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/jackNative.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} $@.d;\
	    $(COMPILE.cc) -g -Werror -D${WITH_JUNIT} -DWITH_JACK -I${JNI_INCLUDE_OS} -I${JNI_INCLUDE_BASE} `pkg-config --cflags jack` -std=c++11   -fPIC  -Dmain=__nomain -MMD -MP -MF $@.d -o ${OBJECTDIR}/jackNative_nomain.o jackNative.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/jackNative.o ${OBJECTDIR}/jackNative_nomain.o;\
	fi

${OBJECTDIR}/windowsNative_nomain.o: ${OBJECTDIR}/windowsNative.o windowsNative.cpp 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/windowsNative.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} $@.d;\
	    $(COMPILE.cc) -g -Werror -D${WITH_JUNIT} -DWITH_JACK -I${JNI_INCLUDE_OS} -I${JNI_INCLUDE_BASE} `pkg-config --cflags jack` -std=c++11   -fPIC  -Dmain=__nomain -MMD -MP -MF $@.d -o ${OBJECTDIR}/windowsNative_nomain.o windowsNative.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/windowsNative.o ${OBJECTDIR}/windowsNative_nomain.o;\
	fi

# Run Test Targets
.test-conf:
	@if [ "${TEST}" = "" ]; \
	then  \
	    ${TESTDIR}/TestFiles/f2 || true; \
	    ${TESTDIR}/TestFiles/f3 || true; \
	    ${TESTDIR}/TestFiles/f1 || true; \
	    ${TESTDIR}/TestFiles/f4 || true; \
	else  \
	    ./${TEST} || true; \
	fi

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libnative.${CND_DLIB_EXT}

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
