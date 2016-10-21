/*----------------------------------------------------------------------------*/
/* Copyright (c) FIRST 2016. All Rights Reserved.                             */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

#include "edu_wpi_cameraserver_CameraServerJNI.h"

#include "llvm/SmallString.h"
#include "support/jni_util.h"

#include "cameraserver_cpp.h"

using namespace wpi::java;

//
// Globals and load/unload
//

// Used for callback.
static JavaVM *jvm = nullptr;
static jclass usbCameraInfoCls = nullptr;
static jclass videoModeCls = nullptr;

extern "C" {

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
  jvm = vm;

  JNIEnv *env;
  if (vm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6) != JNI_OK)
    return JNI_ERR;

  // Cache references to classes
  jclass local;

  local = env->FindClass("edu/wpi/cameraserver/USBCameraInfo");
  if (!local) return JNI_ERR;
  usbCameraInfoCls = static_cast<jclass>(env->NewGlobalRef(local));
  if (!usbCameraInfoCls) return JNI_ERR;
  env->DeleteLocalRef(local);

  local = env->FindClass("edu/wpi/cameraserver/VideoMode");
  if (!local) return JNI_ERR;
  videoModeCls = static_cast<jclass>(env->NewGlobalRef(local));
  if (!videoModeCls) return JNI_ERR;
  env->DeleteLocalRef(local);

  return JNI_VERSION_1_6;
}

JNIEXPORT void JNICALL JNI_OnUnload(JavaVM *vm, void *reserved) {
  JNIEnv *env;
  if (vm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6) != JNI_OK)
    return;
  // Delete global references
  if (usbCameraInfoCls) env->DeleteGlobalRef(usbCameraInfoCls);
  if (videoModeCls) env->DeleteGlobalRef(videoModeCls);
  jvm = nullptr;
}

}  // extern "C"

static void ReportError(JNIEnv *env, CS_Status status, bool do_throw = true) {
  // TODO
}

static inline bool CheckStatus(JNIEnv *env, CS_Status status,
                               bool do_throw = true) {
  if (status != 0) ReportError(env, status, do_throw);
  return status == 0;
}

static jobject MakeJObject(JNIEnv *env, const cs::USBCameraInfo &info) {
  static jmethodID constructor = env->GetMethodID(
      usbCameraInfoCls, "<init>", "(ILjava/lang/String;Ljava/lang/String;)V");
  JLocal<jstring> path(env, MakeJString(env, info.path));
  JLocal<jstring> name(env, MakeJString(env, info.name));
  return env->NewObject(usbCameraInfoCls, constructor,
                        static_cast<jint>(info.dev), path.obj(), name.obj());
}

static jobject MakeJObject(JNIEnv *env, const cs::VideoMode &videoMode) {
  static jmethodID constructor =
      env->GetMethodID(videoModeCls, "<init>", "(IIII)V");
  return env->NewObject(
      videoModeCls, constructor, static_cast<jint>(videoMode.pixelFormat),
      static_cast<jint>(videoMode.width), static_cast<jint>(videoMode.height),
      static_cast<jint>(videoMode.fps));
}

extern "C" {

/*
 * Class:     edu_wpi_cameraserver_CameraServerJNI
 * Method:    getPropertyType
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_edu_wpi_cameraserver_CameraServerJNI_getPropertyType
  (JNIEnv *env, jclass, jint property)
{
  CS_Status status = 0;
  auto val = cs::GetPropertyType(property, &status);
  CheckStatus(env, status);
  return val;
}

/*
 * Class:     edu_wpi_cameraserver_CameraServerJNI
 * Method:    getPropertyName
 * Signature: (I)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_edu_wpi_cameraserver_CameraServerJNI_getPropertyName
  (JNIEnv *env, jclass, jint property)
{
  CS_Status status = 0;
  llvm::SmallString<128> buf;
  auto str = cs::GetPropertyName(property, buf, &status);
  if (!CheckStatus(env, status)) return nullptr;
  return MakeJString(env, str);
}

/*
 * Class:     edu_wpi_cameraserver_CameraServerJNI
 * Method:    getProperty
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_edu_wpi_cameraserver_CameraServerJNI_getProperty
  (JNIEnv *env, jclass, jint property)
{
  CS_Status status = 0;
  auto val = cs::GetProperty(property, &status);
  CheckStatus(env, status);
  return val;
}

/*
 * Class:     edu_wpi_cameraserver_CameraServerJNI
 * Method:    setProperty
 * Signature: (II)V
 */
JNIEXPORT void JNICALL Java_edu_wpi_cameraserver_CameraServerJNI_setProperty
  (JNIEnv *env, jclass, jint property, jint value)
{
  CS_Status status = 0;
  cs::SetProperty(property, value, &status);
  CheckStatus(env, status);
}

/*
 * Class:     edu_wpi_cameraserver_CameraServerJNI
 * Method:    getPropertyMin
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_edu_wpi_cameraserver_CameraServerJNI_getPropertyMin
  (JNIEnv *env, jclass, jint property)
{
  CS_Status status = 0;
  auto val = cs::GetPropertyMin(property, &status);
  CheckStatus(env, status);
  return val;
}

/*
 * Class:     edu_wpi_cameraserver_CameraServerJNI
 * Method:    getPropertyMax
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_edu_wpi_cameraserver_CameraServerJNI_getPropertyMax
  (JNIEnv *env, jclass, jint property)
{
  CS_Status status = 0;
  auto val = cs::GetPropertyMax(property, &status);
  CheckStatus(env, status);
  return val;
}

/*
 * Class:     edu_wpi_cameraserver_CameraServerJNI
 * Method:    getPropertyStep
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_edu_wpi_cameraserver_CameraServerJNI_getPropertyStep
  (JNIEnv *env, jclass, jint property)
{
  CS_Status status = 0;
  auto val = cs::GetPropertyStep(property, &status);
  CheckStatus(env, status);
  return val;
}

/*
 * Class:     edu_wpi_cameraserver_CameraServerJNI
 * Method:    getPropertyDefault
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_edu_wpi_cameraserver_CameraServerJNI_getPropertyDefault
  (JNIEnv *env, jclass, jint property)
{
  CS_Status status = 0;
  auto val = cs::GetPropertyDefault(property, &status);
  CheckStatus(env, status);
  return val;
}

/*
 * Class:     edu_wpi_cameraserver_CameraServerJNI
 * Method:    getStringProperty
 * Signature: (I)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_edu_wpi_cameraserver_CameraServerJNI_getStringProperty
  (JNIEnv *env, jclass, jint property)
{
  CS_Status status = 0;
  llvm::SmallString<128> buf;
  auto str = cs::GetStringProperty(property, buf, &status);
  if (!CheckStatus(env, status)) return nullptr;
  return MakeJString(env, str);
}

/*
 * Class:     edu_wpi_cameraserver_CameraServerJNI
 * Method:    setStringProperty
 * Signature: (ILjava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_edu_wpi_cameraserver_CameraServerJNI_setStringProperty
  (JNIEnv *env, jclass, jint property, jstring value)
{
  CS_Status status = 0;
  cs::SetStringProperty(property, JStringRef{env, value}, &status);
  CheckStatus(env, status);
}

/*
 * Class:     edu_wpi_cameraserver_CameraServerJNI
 * Method:    getEnumPropertyChoices
 * Signature: (I)[Ljava/lang/String;
 */
JNIEXPORT jobjectArray JNICALL Java_edu_wpi_cameraserver_CameraServerJNI_getEnumPropertyChoices
  (JNIEnv *env, jclass, jint property)
{
  CS_Status status = 0;
  auto arr = cs::GetEnumPropertyChoices(property, &status);
  if (!CheckStatus(env, status)) return nullptr;
  return MakeJStringArray(env, arr);
}

/*
 * Class:     edu_wpi_cameraserver_CameraServerJNI
 * Method:    createUSBCameraDev
 * Signature: (Ljava/lang/String;I)I
 */
JNIEXPORT jint JNICALL Java_edu_wpi_cameraserver_CameraServerJNI_createUSBCameraDev
  (JNIEnv *env, jclass, jstring name, jint dev)
{
  CS_Status status = 0;
  auto val = cs::CreateUSBCameraDev(JStringRef{env, name}, dev, &status);
  CheckStatus(env, status);
  return val;
}

/*
 * Class:     edu_wpi_cameraserver_CameraServerJNI
 * Method:    createUSBCameraPath
 * Signature: (Ljava/lang/String;Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_edu_wpi_cameraserver_CameraServerJNI_createUSBCameraPath
  (JNIEnv *env, jclass, jstring name, jstring path)
{
  CS_Status status = 0;
  auto val = cs::CreateUSBCameraPath(JStringRef{env, name},
                                     JStringRef{env, path}, &status);
  CheckStatus(env, status);
  return val;
}

/*
 * Class:     edu_wpi_cameraserver_CameraServerJNI
 * Method:    createHTTPCamera
 * Signature: (Ljava/lang/String;Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_edu_wpi_cameraserver_CameraServerJNI_createHTTPCamera
  (JNIEnv *env, jclass, jstring name, jstring url)
{
  CS_Status status = 0;
  auto val = cs::CreateHTTPCamera(JStringRef{env, name},
                                  JStringRef{env, url}, &status);
  CheckStatus(env, status);
  return val;
}

/*
 * Class:     edu_wpi_cameraserver_CameraServerJNI
 * Method:    createCvSource
 * Signature: (Ljava/lang/String;IIII)I
 */
JNIEXPORT jint JNICALL Java_edu_wpi_cameraserver_CameraServerJNI_createCvSource
  (JNIEnv *env, jclass, jstring name, jint pixelFormat, jint width, jint height,
   jint fps)
{
  CS_Status status = 0;
  auto val = cs::CreateCvSource(
      JStringRef{env, name},
      cs::VideoMode{static_cast<cs::VideoMode::PixelFormat>(pixelFormat),
                    static_cast<int>(width), static_cast<int>(height),
                    static_cast<int>(fps)},
      &status);
  CheckStatus(env, status);
  return val;
}

/*
 * Class:     edu_wpi_cameraserver_CameraServerJNI
 * Method:    getSourceName
 * Signature: (I)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_edu_wpi_cameraserver_CameraServerJNI_getSourceName
  (JNIEnv *env, jclass, jint source)
{
  CS_Status status = 0;
  llvm::SmallString<128> buf;
  auto str = cs::GetSourceName(source, buf, &status);
  if (!CheckStatus(env, status)) return nullptr;
  return MakeJString(env, str);
}

/*
 * Class:     edu_wpi_cameraserver_CameraServerJNI
 * Method:    getSourceDescription
 * Signature: (I)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_edu_wpi_cameraserver_CameraServerJNI_getSourceDescription
  (JNIEnv *env, jclass, jint source)
{
  CS_Status status = 0;
  llvm::SmallString<128> buf;
  auto str = cs::GetSourceDescription(source, buf, &status);
  if (!CheckStatus(env, status)) return nullptr;
  return MakeJString(env, str);
}

/*
 * Class:     edu_wpi_cameraserver_CameraServerJNI
 * Method:    getSourceLastFrameTime
 * Signature: (I)J
 */
JNIEXPORT jlong JNICALL Java_edu_wpi_cameraserver_CameraServerJNI_getSourceLastFrameTime
  (JNIEnv *env, jclass, jint source)
{
  CS_Status status = 0;
  auto val = cs::GetSourceLastFrameTime(source, &status);
  CheckStatus(env, status);
  return val;
}

/*
 * Class:     edu_wpi_cameraserver_CameraServerJNI
 * Method:    isSourceConnected
 * Signature: (I)Z
 */
JNIEXPORT jboolean JNICALL Java_edu_wpi_cameraserver_CameraServerJNI_isSourceConnected
  (JNIEnv *env, jclass, jint source)
{
  CS_Status status = 0;
  auto val = cs::IsSourceConnected(source, &status);
  CheckStatus(env, status);
  return val;
}

/*
 * Class:     edu_wpi_cameraserver_CameraServerJNI
 * Method:    getSourceProperty
 * Signature: (ILjava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_edu_wpi_cameraserver_CameraServerJNI_getSourceProperty
  (JNIEnv *env, jclass, jint source, jstring name)
{
  CS_Status status = 0;
  auto val = cs::GetSourceProperty(source, JStringRef{env, name}, &status);
  CheckStatus(env, status);
  return val;
}

/*
 * Class:     edu_wpi_cameraserver_CameraServerJNI
 * Method:    enumerateSourceProperties
 * Signature: (I)[I
 */
JNIEXPORT jintArray JNICALL Java_edu_wpi_cameraserver_CameraServerJNI_enumerateSourceProperties
  (JNIEnv *env, jclass, jint source)
{
  CS_Status status = 0;
  llvm::SmallVector<CS_Property, 32> buf;
  auto arr = cs::EnumerateSourceProperties(source, buf, &status);
  if (!CheckStatus(env, status)) return nullptr;
  return MakeJIntArray(env, arr);
}

/*
 * Class:     edu_wpi_cameraserver_CameraServerJNI
 * Method:    getSourceVideoMode
 * Signature: (I)Ledu/wpi/cameraserver/VideoMode;
 */
JNIEXPORT jobject JNICALL Java_edu_wpi_cameraserver_CameraServerJNI_getSourceVideoMode
  (JNIEnv *env, jclass, jint source)
{
  CS_Status status = 0;
  auto val = cs::GetSourceVideoMode(source, &status);
  if (!CheckStatus(env, status)) return nullptr;
  return MakeJObject(env, val);
}

/*
 * Class:     edu_wpi_cameraserver_CameraServerJNI
 * Method:    setSourceVideoMode
 * Signature: (IIIII)Z
 */
JNIEXPORT jboolean JNICALL Java_edu_wpi_cameraserver_CameraServerJNI_setSourceVideoMode
  (JNIEnv *env, jclass, jint source, jint pixelFormat, jint width, jint height,
   jint fps)
{
  CS_Status status = 0;
  auto val = cs::SetSourceVideoMode(
      source,
      cs::VideoMode(static_cast<cs::VideoMode::PixelFormat>(pixelFormat), width,
                    height, fps),
      &status);
  CheckStatus(env, status);
  return val;
}

/*
 * Class:     edu_wpi_cameraserver_CameraServerJNI
 * Method:    setSourcePixelFormat
 * Signature: (II)Z
 */
JNIEXPORT jboolean JNICALL Java_edu_wpi_cameraserver_CameraServerJNI_setSourcePixelFormat
  (JNIEnv *env, jclass, jint source, jint pixelFormat)
{
  CS_Status status = 0;
  auto val = cs::SetSourcePixelFormat(
      source, static_cast<cs::VideoMode::PixelFormat>(pixelFormat), &status);
  CheckStatus(env, status);
  return val;
}

/*
 * Class:     edu_wpi_cameraserver_CameraServerJNI
 * Method:    setSourceResolution
 * Signature: (III)Z
 */
JNIEXPORT jboolean JNICALL Java_edu_wpi_cameraserver_CameraServerJNI_setSourceResolution
  (JNIEnv *env, jclass, jint source, jint width, jint height)
{
  CS_Status status = 0;
  auto val = cs::SetSourceResolution(source, width, height, &status);
  CheckStatus(env, status);
  return val;
}

/*
 * Class:     edu_wpi_cameraserver_CameraServerJNI
 * Method:    setSourceFPS
 * Signature: (II)Z
 */
JNIEXPORT jboolean JNICALL Java_edu_wpi_cameraserver_CameraServerJNI_setSourceFPS
  (JNIEnv *env, jclass, jint source, jint fps)
{
  CS_Status status = 0;
  auto val = cs::SetSourceFPS(source, fps, &status);
  CheckStatus(env, status);
  return val;
}

/*
 * Class:     edu_wpi_cameraserver_CameraServerJNI
 * Method:    enumerateSourceVideoModes
 * Signature: (I)[Ledu/wpi/cameraserver/VideoMode;
 */
JNIEXPORT jobjectArray JNICALL Java_edu_wpi_cameraserver_CameraServerJNI_enumerateSourceVideoModes
  (JNIEnv *env, jclass, jint source)
{
  CS_Status status = 0;
  auto arr = cs::EnumerateSourceVideoModes(source, &status);
  if (!CheckStatus(env, status)) return nullptr;
  jobjectArray jarr =
      env->NewObjectArray(arr.size(), videoModeCls, nullptr);
  if (!jarr) return nullptr;
  for (size_t i = 0; i < arr.size(); ++i) {
    JLocal<jobject> jelem{env, MakeJObject(env, arr[i])};
    env->SetObjectArrayElement(jarr, i, jelem);
  }
  return jarr;
}

/*
 * Class:     edu_wpi_cameraserver_CameraServerJNI
 * Method:    copySource
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_edu_wpi_cameraserver_CameraServerJNI_copySource
  (JNIEnv *env, jclass, jint source)
{
  CS_Status status = 0;
  auto val = cs::CopySource(source, &status);
  CheckStatus(env, status);
  return val;
}

/*
 * Class:     edu_wpi_cameraserver_CameraServerJNI
 * Method:    releaseSource
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_edu_wpi_cameraserver_CameraServerJNI_releaseSource
  (JNIEnv *env, jclass, jint source)
{
  CS_Status status = 0;
  cs::ReleaseSource(source, &status);
  CheckStatus(env, status);
}

/*
 * Class:     edu_wpi_cameraserver_CameraServerJNI
 * Method:    putSourceFrame
 * Signature: (IJ)V
 */
JNIEXPORT void JNICALL Java_edu_wpi_cameraserver_CameraServerJNI_putSourceFrame
  (JNIEnv *env, jclass, jint source, jlong imageNativeObj)
{
  cv::Mat& image = *((cv::Mat*)imageNativeObj);
  CS_Status status = 0;
  cs::PutSourceFrame(source, image, &status);
  CheckStatus(env, status);
}

/*
 * Class:     edu_wpi_cameraserver_CameraServerJNI
 * Method:    notifySourceError
 * Signature: (ILjava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_edu_wpi_cameraserver_CameraServerJNI_notifySourceError
  (JNIEnv *env, jclass, jint source, jstring msg)
{
  CS_Status status = 0;
  cs::NotifySourceError(source, JStringRef{env, msg}, &status);
  CheckStatus(env, status);
}

/*
 * Class:     edu_wpi_cameraserver_CameraServerJNI
 * Method:    setSourceConnected
 * Signature: (IZ)V
 */
JNIEXPORT void JNICALL Java_edu_wpi_cameraserver_CameraServerJNI_setSourceConnected
  (JNIEnv *env, jclass, jint source, jboolean connected)
{
  CS_Status status = 0;
  cs::SetSourceConnected(source, connected, &status);
  CheckStatus(env, status);
}

/*
 * Class:     edu_wpi_cameraserver_CameraServerJNI
 * Method:    createSourceProperty
 * Signature: (ILjava/lang/String;I)I
 */
JNIEXPORT jint JNICALL Java_edu_wpi_cameraserver_CameraServerJNI_createSourceProperty
  (JNIEnv *env, jclass, jint source, jstring name, jint type)
{
  CS_Status status = 0;
  auto val =
      cs::CreateSourceProperty(source, JStringRef{env, name},
                               static_cast<CS_PropertyType>(type), &status);
  CheckStatus(env, status);
  return val;
}

/*
 * Class:     edu_wpi_cameraserver_CameraServerJNI
 * Method:    removeSourceProperty
 * Signature: (II)V
 */
JNIEXPORT void JNICALL Java_edu_wpi_cameraserver_CameraServerJNI_removeSourceProperty
  (JNIEnv *env, jclass, jint source, jint property)
{
  CS_Status status = 0;
  cs::RemoveSourceProperty(source, property, &status);
  CheckStatus(env, status);
}

/*
 * Class:     edu_wpi_cameraserver_CameraServerJNI
 * Method:    removeSourcePropertyByName
 * Signature: (ILjava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_edu_wpi_cameraserver_CameraServerJNI_removeSourcePropertyByName
  (JNIEnv *env, jclass, jint source, jstring name)
{
  CS_Status status = 0;
  cs::RemoveSourceProperty(source, JStringRef{env, name}, &status);
  CheckStatus(env, status);
}

/*
 * Class:     edu_wpi_cameraserver_CameraServerJNI
 * Method:    createMJPEGServer
 * Signature: (Ljava/lang/String;Ljava/lang/String;I)I
 */
JNIEXPORT jint JNICALL Java_edu_wpi_cameraserver_CameraServerJNI_createMJPEGServer
  (JNIEnv *env, jclass, jstring name, jstring listenAddress, jint port)
{
  CS_Status status = 0;
  auto val = cs::CreateMJPEGServer(
      JStringRef{env, name}, JStringRef{env, listenAddress}, port, &status);
  CheckStatus(env, status);
  return val;
}

/*
 * Class:     edu_wpi_cameraserver_CameraServerJNI
 * Method:    createCvSink
 * Signature: (Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_edu_wpi_cameraserver_CameraServerJNI_createCvSink
  (JNIEnv *env, jclass, jstring name)
{
  CS_Status status = 0;
  auto val = cs::CreateCvSink(JStringRef{env, name}, &status);
  CheckStatus(env, status);
  return val;
}

/*
 * Class:     edu_wpi_cameraserver_CameraServerJNI
 * Method:    getSinkName
 * Signature: (I)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_edu_wpi_cameraserver_CameraServerJNI_getSinkName
  (JNIEnv *env, jclass, jint sink)
{
  CS_Status status = 0;
  llvm::SmallString<128> buf;
  auto str = cs::GetSinkName(sink, buf, &status);
  if (!CheckStatus(env, status)) return nullptr;
  return MakeJString(env, str);
}

/*
 * Class:     edu_wpi_cameraserver_CameraServerJNI
 * Method:    getSinkDescription
 * Signature: (I)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_edu_wpi_cameraserver_CameraServerJNI_getSinkDescription
  (JNIEnv *env, jclass, jint sink)
{
  CS_Status status = 0;
  llvm::SmallString<128> buf;
  auto str = cs::GetSinkDescription(sink, buf, &status);
  if (!CheckStatus(env, status)) return nullptr;
  return MakeJString(env, str);
}

/*
 * Class:     edu_wpi_cameraserver_CameraServerJNI
 * Method:    setSinkSource
 * Signature: (II)V
 */
JNIEXPORT void JNICALL Java_edu_wpi_cameraserver_CameraServerJNI_setSinkSource
  (JNIEnv *env, jclass, jint sink, jint source)
{
  CS_Status status = 0;
  cs::SetSinkSource(sink, source, &status);
  CheckStatus(env, status);
}

/*
 * Class:     edu_wpi_cameraserver_CameraServerJNI
 * Method:    getSinkSourceProperty
 * Signature: (ILjava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_edu_wpi_cameraserver_CameraServerJNI_getSinkSourceProperty
  (JNIEnv *env, jclass, jint sink, jstring name)
{
  CS_Status status = 0;
  auto val = cs::GetSinkSourceProperty(sink, JStringRef{env, name}, &status);
  CheckStatus(env, status);
  return val;
}

/*
 * Class:     edu_wpi_cameraserver_CameraServerJNI
 * Method:    getSinkSource
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_edu_wpi_cameraserver_CameraServerJNI_getSinkSource
  (JNIEnv *env, jclass, jint sink)
{
  CS_Status status = 0;
  auto val = cs::GetSinkSource(sink, &status);
  CheckStatus(env, status);
  return val;
}

/*
 * Class:     edu_wpi_cameraserver_CameraServerJNI
 * Method:    copySink
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_edu_wpi_cameraserver_CameraServerJNI_copySink
  (JNIEnv *env, jclass, jint sink)
{
  CS_Status status = 0;
  auto val = cs::CopySink(sink, &status);
  CheckStatus(env, status);
  return val;
}

/*
 * Class:     edu_wpi_cameraserver_CameraServerJNI
 * Method:    releaseSink
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_edu_wpi_cameraserver_CameraServerJNI_releaseSink
  (JNIEnv *env, jclass, jint sink)
{
  CS_Status status = 0;
  cs::ReleaseSink(sink, &status);
  CheckStatus(env, status);
}

/*
 * Class:     edu_wpi_cameraserver_CameraServerJNI
 * Method:    grabSinkFrame
 * Signature: (IJ)J
 */
JNIEXPORT jlong JNICALL Java_edu_wpi_cameraserver_CameraServerJNI_grabSinkFrame
  (JNIEnv *env, jclass, jint sink, jlong imageNativeObj)
{
  cv::Mat& image = *((cv::Mat*)imageNativeObj);
  CS_Status status = 0;
  auto rv = cs::GrabSinkFrame(sink, image, &status);
  CheckStatus(env, status);
  return rv;
}

/*
 * Class:     edu_wpi_cameraserver_CameraServerJNI
 * Method:    getSinkError
 * Signature: (I)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_edu_wpi_cameraserver_CameraServerJNI_getSinkError
  (JNIEnv *env, jclass, jint sink)
{
  CS_Status status = 0;
  llvm::SmallString<128> buf;
  auto str = cs::GetSinkError(sink, buf, &status);
  if (!CheckStatus(env, status)) return nullptr;
  return MakeJString(env, str);
}

/*
 * Class:     edu_wpi_cameraserver_CameraServerJNI
 * Method:    setSinkEnabled
 * Signature: (IZ)V
 */
JNIEXPORT void JNICALL Java_edu_wpi_cameraserver_CameraServerJNI_setSinkEnabled
  (JNIEnv *env, jclass, jint sink, jboolean enabled)
{
  CS_Status status = 0;
  cs::SetSinkEnabled(sink, enabled, &status);
  CheckStatus(env, status);
}

/*
 * Class:     edu_wpi_cameraserver_CameraServerJNI
 * Method:    removeSourceListener
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_edu_wpi_cameraserver_CameraServerJNI_removeSourceListener
  (JNIEnv *env, jclass, jint handle)
{
  CS_Status status = 0;
  cs::RemoveSourceListener(handle, &status);
  CheckStatus(env, status);
}

/*
 * Class:     edu_wpi_cameraserver_CameraServerJNI
 * Method:    removeSinkListener
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_edu_wpi_cameraserver_CameraServerJNI_removeSinkListener
  (JNIEnv *env, jclass, jint handle)
{
  CS_Status status = 0;
  cs::RemoveSinkListener(handle, &status);
  CheckStatus(env, status);
}

/*
 * Class:     edu_wpi_cameraserver_CameraServerJNI
 * Method:    enumerateUSBCameras
 * Signature: ()[Ledu/wpi/cameraserver/USBCameraInfo;
 */
JNIEXPORT jobjectArray JNICALL Java_edu_wpi_cameraserver_CameraServerJNI_enumerateUSBCameras
  (JNIEnv *env, jclass)
{
  CS_Status status = 0;
  auto arr = cs::EnumerateUSBCameras(&status);
  if (!CheckStatus(env, status)) return nullptr;
  jobjectArray jarr =
      env->NewObjectArray(arr.size(), usbCameraInfoCls, nullptr);
  if (!jarr) return nullptr;
  for (size_t i = 0; i < arr.size(); ++i) {
    JLocal<jobject> jelem{env, MakeJObject(env, arr[i])};
    env->SetObjectArrayElement(jarr, i, jelem);
  }
  return jarr;
}

/*
 * Class:     edu_wpi_cameraserver_CameraServerJNI
 * Method:    enumerateSources
 * Signature: ()[I
 */
JNIEXPORT jintArray JNICALL Java_edu_wpi_cameraserver_CameraServerJNI_enumerateSources
  (JNIEnv *env, jclass)
{
  CS_Status status = 0;
  llvm::SmallVector<CS_Source, 16> buf;
  auto arr = cs::EnumerateSourceHandles(buf, &status);
  if (!CheckStatus(env, status)) return nullptr;
  return MakeJIntArray(env, arr);
}

/*
 * Class:     edu_wpi_cameraserver_CameraServerJNI
 * Method:    enumerateSinks
 * Signature: ()[I
 */
JNIEXPORT jintArray JNICALL Java_edu_wpi_cameraserver_CameraServerJNI_enumerateSinks
  (JNIEnv *env, jclass)
{
  CS_Status status = 0;
  llvm::SmallVector<CS_Sink, 16> buf;
  auto arr = cs::EnumerateSinkHandles(buf, &status);
  if (!CheckStatus(env, status)) return nullptr;
  return MakeJIntArray(env, arr);
}

}  // extern "C"
