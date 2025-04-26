README.txt
Roger B. Dannenberg
16 Jun 2009
updated 2021

This directory implements a JNI library so that Java programs can use
the PortMidi API. This was mainly created to implement PmDefaults, a
program to set default input and output devices for PortMidi
applications.

PmDefaults never found much use. I recommend you implement
per-application preferences and store default PortMidi device
numbers for input and output there. (Or better yet, store
device *names* since numbers can change if you plug in or 
remove USB devices.) 

Even without PmDefaults, a PortMidi API for Java is probably an
improvement over other Java libraries, but there is very little MIDI
development in Java, so I have not maintained this API. The only thing
probably seriously wrong now is an interface to the
Pm_CreateVirtualInput and Pm_CreateVirtualOutput functions, which are
new additions.

I will leave the code here, and if there is a demand, please either
update it or let your needs be known. Perhaps I or someone can help.
 
==================================================================

BUILDING PmDefaults PROGRAM

You must have a JDK installed (Java development kit including javac
(the Java compiler), jni.h, etc.

Test java on the command line, e.g., type: javac -version

Enable these options in the main CMakeLists.txt file (run CMake
from your top-level repository directory):
     BUILD_JAVA_NATIVE_INTERFACE
     BUILD_PMDEFAULTS
In my Ubuntu linux with jdk-15, ccmake was unable to find my JDK, so
I have to manually set CMake variables as follows (type 't' to see
these in ccmake):
    JAVA_AWT_INCLUDE_PATH  /usr/lib/jvm/jdk-15/include
    JAVA_AWT_LIBRARY       /usr/lib/jvm/jdk-15/lib
    JAVA_INCLUDE_PATH      /usr/lib/jvm/jdk-15/include
    JAVA_INCLUDE_PATH2     /usr/lib/jvm/jdk-15/include
    JAVA_JVM_LIBRARY       /usr/lib/jvm/jdk-15/lib
Of course, your paths may differ.
    

RUNNING PmDefaults PROGRAM

After building the pmdefaults target with make, Visual Studio, or Xcode:
In Windows:
   [from the command line:]
   cd portmidi\pm_java\pmdefaults  -- change to this directory
   pmdefaults                      -- runs pmdefaults.bat
   [or from the finder:]
   double-click on pmdefaults.bat
In macOS and Linux:
   cd portmidi\pm_java\pmdefaults -- change to this directory
   ./pmdefaults                   -- shell script to invoke java


---- old implementation notes ----

For Windows, we use the free software JavaExe.exe. The copy here was
downloaded from 

http://software.techrepublic.com.com/abstract.aspx?kw=javaexe&docid=767485

I found this page by visiting http://software.techrepublic.com.com and
searching in the "Software" category for "JavaExe"

JavaExe works by placing the JavaExe.exe file in the directory with the
Java application jar file and then *renaming* JavaExe.exe to the name
of the jar file, but keeping the .exe extension. (See make.bat for this 
step.) Documentation for JavaExe can be obtained by downloading the
whole program from the URL(s) above.
