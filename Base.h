#pragma once
#ifndef BASE_H
#define BASE_H

#ifndef MATH_H
#include "Math.h"
#endif

#define ECODE_OK 0x00
#define ECODE_UNDEF 0x01
#define ECODE_ABORT 0x02
#define ECODE_NOTABORT 0xFD
#define ECODE_FAIL 0x03
#define ECODE_NOTFAIL 0xFC
#define ECODE_USERERR_ABORT 0x06
#define ECODE_MEMERR_FAIL 0x0b


#define BASE_STATE_NEW 0x00
#define BASE_STATE_INIT 0x01
#define BASE_STATE_INIT_NOT 0xfe
#define BASE_STATE_OPEN 0x02
#define BASE_STATE_OPEN_NOT 0xfd



class Base {
public:
	Base();
	~Base();

	inline bool Err(unsigned char ecode) { return (ecode != ECODE_OK); }
	inline bool RetOk(const unsigned char err_code) const { return (err_code == ECODE_OK); }
	inline bool IsErrFail(const unsigned char err_code) const { return ((err_code&ECODE_FAIL) == ECODE_FAIL); }
	inline bool IsUserErrAbort(const unsigned char err_code) const { return ((err_code&(ECODE_USERERR_ABORT | ECODE_FAIL)) == ECODE_USERERR_ABORT); }
	void SetFailErr(const unsigned char sub_err_code) { m_obj_err = m_obj_err | (sub_err_code | ECODE_FAIL); }
	void SetAbortErr(const unsigned char sub_err_code) { m_obj_err = m_obj_err | (sub_err_code | ECODE_ABORT); }
	inline unsigned char GetFailErr() { return (m_obj_err & ECODE_NOTFAIL); }
	inline unsigned char GetAbortErr() { return (m_obj_err&ECODE_NOTABORT); }
	inline bool IsFailSubErr(const unsigned char sub_err_code) { return ((sub_err_code & ECODE_NOTFAIL) == sub_err_code); }
	inline bool IsAbortSubErr(const unsigned char sub_err_code) { return ((sub_err_code & ECODE_NOTABORT) == sub_err_code); }

	inline bool IsStateInitialized() const { return ((BASE_STATE_INIT & m_obj_state) == BASE_STATE_INIT); }
	inline bool IsStateOpen() const { return ((m_obj_state & BASE_STATE_OPEN) == BASE_STATE_OPEN); }
protected:

	unsigned char m_obj_state;
	unsigned char m_obj_err;

	inline bool IsStateNew() const { return ((BASE_STATE_NEW | m_obj_state) == BASE_STATE_NEW); }

	inline void SetStateNew() { m_obj_state = BASE_STATE_NEW; }
	inline void SetStateInitialized() { m_obj_state = (m_obj_state | BASE_STATE_INIT); }
	inline void SetStateOpen() { m_obj_state = BASE_STATE_OPEN | m_obj_state; }
	inline void SetStateClosed() { m_obj_state = (m_obj_state & BASE_STATE_OPEN_NOT); }

};
#endif
