/*
  OpenMW - The completely unofficial reimplementation of Morrowind
  Copyright (C) 2008  Nicolay Korslund
  Email: < korslund@gmail.com >
  WWW: http://openmw.snaptoad.com/

  This file (al.d) is part of the OpenMW package.

  OpenMW is distributed as free software: you can redistribute it
  and/or modify it under the terms of the GNU General Public License
  version 3, as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  version 3 along with this program. If not, see
  http://www.gnu.org/licenses/ .

 */

module sound.al;

extern(C):

//Defines
const int AL_NONE                                =  0;

const int AL_FALSE                               =  0;
const int AL_TRUE                                =  1;

const int AL_SOURCE_RELATIVE                     =  0x202;
const int AL_CONE_INNER_ANGLE                    =  0x1001;
const int AL_CONE_OUTER_ANGLE                    =  0x1002;
const int AL_PITCH                               =  0x1003;
const int AL_POSITION                            =  0x1004;
const int AL_DIRECTION                           =  0x1005;
const int AL_VELOCITY                            =  0x1006;
const int AL_LOOPING                             =  0x1007;
const int AL_BUFFER                              =  0x1009;
const int AL_GAIN                                =  0x100A;
const int AL_MIN_GAIN                            =  0x100D;
const int AL_MAX_GAIN                            =  0x100E;
const int AL_ORIENTATION                         =  0x100F;
const int AL_SOURCE_STATE                        =  0x1010;
const int AL_INITIAL                             =  0x1011;
const int AL_PLAYING                             =  0x1012;
const int AL_PAUSED                              =  0x1013;
const int AL_STOPPED                             =  0x1014;
const int AL_BUFFERS_QUEUED                      =  0x1015;
const int AL_BUFFERS_PROCESSED                   =  0x1016;
const int AL_SEC_OFFSET                          =  0x1024;
const int AL_SAMPLE_OFFSET                       =  0x1025;
const int AL_BYTE_OFFSET                         =  0x1026;
const int AL_SOURCE_TYPE                         =  0x1027;
const int AL_STATIC                              =  0x1028;
const int AL_STREAMING                           =  0x1029;
const int AL_UNDETERMINED                        =  0x1030;

const int AL_FORMAT_MONO8                        =  0x1100;
const int AL_FORMAT_MONO16                       =  0x1101;
const int AL_FORMAT_STEREO8                      =  0x1102;
const int AL_FORMAT_STEREO16                     =  0x1103;

const int AL_REFERENCE_DISTANCE                  =  0x1020;
const int AL_ROLLOFF_FACTOR                      =  0x1021;
const int AL_CONE_OUTER_GAIN                     =  0x1022;
const int AL_MAX_DISTANCE                        =  0x1023;
const int AL_FREQUENCY                           =  0x2001;
const int AL_BITS                                =  0x2002;
const int AL_CHANNELS                            =  0x2003;
const int AL_SIZE                                =  0x2004;
const int AL_UNUSED                              =  0x2010;
const int AL_PENDING                             =  0x2011;
const int AL_PROCESSED                           =  0x2012;

const int AL_NO_ERROR                            =  AL_FALSE;
const int AL_INVALID_NAME                        =  0xA001;
const int AL_INVALID_ENUM                        =  0xA002;
const int AL_INVALID_VALUE                       =  0xA003;
const int AL_INVALID_OPERATION                   =  0xA004;
const int AL_OUT_OF_MEMORY                       =  0xA005;

const int AL_VENDOR                              =  0xB001;
const int AL_VERSION                             =  0xB002;
const int AL_RENDERER                            =  0xB003;
const int AL_EXTENSIONS                          =  0xB004;

const int AL_DOPPLER_FACTOR                      =  0xC000;
const int AL_DOPPLER_VELOCITY                    =  0xC001;
const int AL_SPEED_OF_SOUND                      =  0xC003;
 
const int AL_DISTANCE_MODEL                      =  0xD000;
const int AL_INVERSE_DISTANCE                    =  0xD001;
const int AL_INVERSE_DISTANCE_CLAMPED            =  0xD002;
const int AL_LINEAR_DISTANCE                     =  0xD003;
const int AL_LINEAR_DISTANCE_CLAMPED             =  0xD004;
const int AL_EXPONENT_DISTANCE                   =  0xD005;
const int AL_EXPONENT_DISTANCE_CLAMPED           =  0xD006;

//Typedefs
alias char ALboolean;
alias char ALchar;
alias char ALbyte;
alias ubyte ALubyte;
alias short ALshort;
alias ushort ALushort;
alias int ALint;
alias uint ALuint;
alias int ALsizei;
alias int ALenum;
alias float ALfloat;
alias double ALdouble;
alias void ALvoid;

//AL Functions
void alEnable( ALenum capability );
void alDisable( ALenum capability );
ALboolean alIsEnabled( ALenum capability );
ALchar* alGetString( ALenum param );
void alGetBooleanv( ALenum param, ALboolean* data );
void alGetIntegerv( ALenum param, ALint* data );
void alGetFloatv( ALenum param, ALfloat* data );
void alGetDoublev( ALenum param, ALdouble* data );
ALboolean alGetBoolean( ALenum param );
ALint alGetInteger( ALenum param );
ALfloat alGetFloat( ALenum param );
ALdouble alGetDouble( ALenum param );
ALenum alGetError();
ALboolean alIsExtensionPresent( ALchar* extname );
void* alGetProcAddress( ALchar* fname );
ALenum alGetEnumValue( ALchar* ename );

void alListenerf( ALenum param, ALfloat value );
void alListener3f( ALenum param, ALfloat value1, ALfloat value2, ALfloat value3 );
void alListenerfv( ALenum param, ALfloat* values );
void alListeneri( ALenum param, ALint value );
void alListener3i( ALenum param, ALint value1, ALint value2, ALint value3 );
void alListeneriv( ALenum param, ALint* values );

void alGetListenerf( ALenum param, ALfloat* value );
void alGetListener3f( ALenum param, ALfloat *value1, ALfloat *value2, ALfloat *value3 );
void alGetListenerfv( ALenum param, ALfloat* values );
void alGetListeneri( ALenum param, ALint* value );
void alGetListener3i( ALenum param, ALint *value1, ALint *value2, ALint *value3 );
void alGetListeneriv( ALenum param, ALint* values );

void alGenSources( ALsizei n, ALuint* sources );
void alDeleteSources( ALsizei n, ALuint* sources );
ALboolean alIsSource( ALuint sid );
void alSourcef( ALuint sid, ALenum param, ALfloat value );
void alSource3f( ALuint sid, ALenum param, ALfloat value1, ALfloat value2, ALfloat value3 );
void alSourcefv( ALuint sid, ALenum param, ALfloat* values );
void alSourcei( ALuint sid, ALenum param, ALint value );
void alSource3i( ALuint sid, ALenum param, ALint value1, ALint value2, ALint value3 );
void alSourceiv( ALuint sid, ALenum param, ALint* values );
void alGetSourcef( ALuint sid, ALenum param, ALfloat* value );
void alGetSource3f( ALuint sid, ALenum param, ALfloat* value1, ALfloat* value2, ALfloat* value3);
void alGetSourcefv( ALuint sid, ALenum param, ALfloat* values );
void alGetSourcei( ALuint sid, ALenum param, ALint* value );
void alGetSource3i( ALuint sid, ALenum param, ALint* value1, ALint* value2, ALint* value3);
void alGetSourceiv( ALuint sid, ALenum param, ALint* values );
void alSourcePlayv( ALsizei ns, ALuint *sids );
void alSourceStopv( ALsizei ns, ALuint *sids );
void alSourceRewindv( ALsizei ns, ALuint *sids );
void alSourcePausev( ALsizei ns, ALuint *sids );
void alSourcePlay( ALuint sid );
void alSourceStop( ALuint sid );
void alSourceRewind( ALuint sid );
void alSourcePause( ALuint sid );
void alSourceQueueBuffers( ALuint sid, ALsizei numEntries, ALuint *bids );
void alSourceUnqueueBuffers( ALuint sid, ALsizei numEntries, ALuint *bids );

void alGenBuffers( ALsizei n, ALuint* buffers );
void alDeleteBuffers( ALsizei n, ALuint* buffers );
ALboolean alIsBuffer( ALuint bid );
void alBufferData( ALuint bid, ALenum format, ALvoid* data, ALsizei size, ALsizei freq );
void alBufferf( ALuint bid, ALenum param, ALfloat value );
void alBuffer3f( ALuint bid, ALenum param, ALfloat value1, ALfloat value2, ALfloat value3 );
void alBufferfv( ALuint bid, ALenum param, ALfloat* values );
void alBufferi( ALuint bid, ALenum param, ALint value );
void alBuffer3i( ALuint bid, ALenum param, ALint value1, ALint value2, ALint value3 );
void alBufferiv( ALuint bid, ALenum param, ALint* values );
void alGetBufferf( ALuint bid, ALenum param, ALfloat* value );
void alGetBuffer3f( ALuint bid, ALenum param, ALfloat* value1, ALfloat* value2, ALfloat* value3);
void alGetBufferfv( ALuint bid, ALenum param, ALfloat* values );
void alGetBufferi( ALuint bid, ALenum param, ALint* value );
void alGetBuffer3i( ALuint bid, ALenum param, ALint* value1, ALint* value2, ALint* value3);
void alGetBufferiv( ALuint bid, ALenum param, ALint* values );
void alDopplerFactor( ALfloat value );
void alDopplerVelocity( ALfloat value );
void alSpeedOfSound( ALfloat value );
void alDistanceModel( ALenum distanceModel );

typedef void ( *LPALENABLE)( ALenum capability );
typedef void ( *LPALDISABLE)( ALenum capability );
typedef ALboolean ( *LPALISENABLED)( ALenum capability );
typedef const ALchar* ( *LPALGETSTRING)( ALenum param );
typedef void ( *LPALGETBOOLEANV)( ALenum param, ALboolean* data );
typedef void ( *LPALGETINTEGERV)( ALenum param, ALint* data );
typedef void ( *LPALGETFLOATV)( ALenum param, ALfloat* data );
typedef void ( *LPALGETDOUBLEV)( ALenum param, ALdouble* data );
typedef ALboolean ( *LPALGETBOOLEAN)( ALenum param );
typedef ALint ( *LPALGETINTEGER)( ALenum param );
typedef ALfloat ( *LPALGETFLOAT)( ALenum param );
typedef ALdouble ( *LPALGETDOUBLE)( ALenum param );
typedef ALenum ( *LPALGETERROR)();
typedef ALboolean ( *LPALISEXTENSIONPRESENT)(ALchar* extname );
typedef void* ( *LPALGETPROCADDRESS)( ALchar* fname );
typedef ALenum ( *LPALGETENUMVALUE)( ALchar* ename );
typedef void ( *LPALLISTENERF)( ALenum param, ALfloat value );
typedef void ( *LPALLISTENER3F)( ALenum param, ALfloat value1, ALfloat value2, ALfloat value3 );
typedef void ( *LPALLISTENERFV)( ALenum param, ALfloat* values );
typedef void ( *LPALLISTENERI)( ALenum param, ALint value );
typedef void ( *LPALLISTENER3I)( ALenum param, ALint value1, ALint value2, ALint value3 );
typedef void ( *LPALLISTENERIV)( ALenum param, ALint* values );
typedef void ( *LPALGETLISTENERF)( ALenum param, ALfloat* value );
typedef void ( *LPALGETLISTENER3F)( ALenum param, ALfloat *value1, ALfloat *value2, ALfloat *value3 );
typedef void ( *LPALGETLISTENERFV)( ALenum param, ALfloat* values );
typedef void ( *LPALGETLISTENERI)( ALenum param, ALint* value );
typedef void ( *LPALGETLISTENER3I)( ALenum param, ALint *value1, ALint *value2, ALint *value3 );
typedef void ( *LPALGETLISTENERIV)( ALenum param, ALint* values );
typedef void ( *LPALGENSOURCES)( ALsizei n, ALuint* sources );
typedef void ( *LPALDELETESOURCES)( ALsizei n, ALuint* sources );
typedef ALboolean ( *LPALISSOURCE)( ALuint sid );
typedef void ( *LPALSOURCEF)( ALuint sid, ALenum param, ALfloat value);
typedef void ( *LPALSOURCE3F)( ALuint sid, ALenum param, ALfloat value1, ALfloat value2, ALfloat value3 );
typedef void ( *LPALSOURCEFV)( ALuint sid, ALenum param, ALfloat* values );
typedef void ( *LPALSOURCEI)( ALuint sid, ALenum param, ALint value);
typedef void ( *LPALSOURCE3I)( ALuint sid, ALenum param, ALint value1, ALint value2, ALint value3 );
typedef void ( *LPALSOURCEIV)( ALuint sid, ALenum param, ALint* values );
typedef void ( *LPALGETSOURCEF)( ALuint sid, ALenum param, ALfloat* value );
typedef void ( *LPALGETSOURCE3F)( ALuint sid, ALenum param, ALfloat* value1, ALfloat* value2, ALfloat* value3);
typedef void ( *LPALGETSOURCEFV)( ALuint sid, ALenum param, ALfloat* values );
typedef void ( *LPALGETSOURCEI)( ALuint sid, ALenum param, ALint* value );
typedef void ( *LPALGETSOURCE3I)( ALuint sid, ALenum param, ALint* value1, ALint* value2, ALint* value3);
typedef void ( *LPALGETSOURCEIV)( ALuint sid, ALenum param, ALint* values );
typedef void ( *LPALSOURCEPLAYV)( ALsizei ns, ALuint *sids );
typedef void ( *LPALSOURCESTOPV)( ALsizei ns, ALuint *sids );
typedef void ( *LPALSOURCEREWINDV)( ALsizei ns, ALuint *sids );
typedef void ( *LPALSOURCEPAUSEV)( ALsizei ns, ALuint *sids );
typedef void ( *LPALSOURCEPLAY)( ALuint sid );
typedef void ( *LPALSOURCESTOP)( ALuint sid );
typedef void ( *LPALSOURCEREWIND)( ALuint sid );
typedef void ( *LPALSOURCEPAUSE)( ALuint sid );
typedef void ( *LPALSOURCEQUEUEBUFFERS)(ALuint sid, ALsizei numEntries, ALuint *bids );
typedef void ( *LPALSOURCEUNQUEUEBUFFERS)(ALuint sid, ALsizei numEntries, ALuint *bids );
typedef void ( *LPALGENBUFFERS)( ALsizei n, ALuint* buffers );
typedef void ( *LPALDELETEBUFFERS)( ALsizei n, ALuint* buffers );
typedef ALboolean ( *LPALISBUFFER)( ALuint bid );
typedef void ( *LPALBUFFERDATA)( ALuint bid, ALenum format, ALvoid* data, ALsizei size, ALsizei freq );
typedef void ( *LPALBUFFERF)( ALuint bid, ALenum param, ALfloat value);
typedef void ( *LPALBUFFER3F)( ALuint bid, ALenum param, ALfloat value1, ALfloat value2, ALfloat value3 );
typedef void ( *LPALBUFFERFV)( ALuint bid, ALenum param, ALfloat* values );
typedef void ( *LPALBUFFERI)( ALuint bid, ALenum param, ALint value);
typedef void ( *LPALBUFFER3I)( ALuint bid, ALenum param, ALint value1, ALint value2, ALint value3 );
typedef void ( *LPALBUFFERIV)( ALuint bid, ALenum param, ALint* values );
typedef void ( *LPALGETBUFFERF)( ALuint bid, ALenum param, ALfloat* value );
typedef void ( *LPALGETBUFFER3F)( ALuint bid, ALenum param, ALfloat* value1, ALfloat* value2, ALfloat* value3);
typedef void ( *LPALGETBUFFERFV)( ALuint bid, ALenum param, ALfloat* values );
typedef void ( *LPALGETBUFFERI)( ALuint bid, ALenum param, ALint* value );
typedef void ( *LPALGETBUFFER3I)( ALuint bid, ALenum param, ALint* value1, ALint* value2, ALint* value3);
typedef void ( *LPALGETBUFFERIV)( ALuint bid, ALenum param, ALint* values );
typedef void ( *LPALDOPPLERFACTOR)( ALfloat value );
typedef void ( *LPALDOPPLERVELOCITY)( ALfloat value );
typedef void ( *LPALSPEEDOFSOUND)( ALfloat value );
typedef void ( *LPALDISTANCEMODEL)( ALenum distanceModel );
