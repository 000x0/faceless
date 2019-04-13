#pragma once
#include "..\Utils\Color.h"
#include "..\Utils\Utils.h"
#define _CRT_SECURE_NO_WARNINGS

class convar;
using fn_change_callback_t = void( *)(convar* var, const char* pOldValue, float flOldValue);

inline int UtlMemory_CalcNewAllocationCount( int nAllocationCount, int nGrowSize, int nNewSize, int nBytesItem ) {
	if (nGrowSize)
		nAllocationCount = ((1 + ((nNewSize - 1) / nGrowSize)) * nGrowSize);
	else {
		if (!nAllocationCount)
			nAllocationCount = (31 + nBytesItem) / nBytesItem;

		while (nAllocationCount < nNewSize)
			nAllocationCount *= 2;
	}

	return nAllocationCount;
}

template< class T, class I = int >
class CUtlMemory {
public:
	T & operator[]( I i ) {
		return m_pMemory[i];
	}

	T* Base() {
		return m_pMemory;
	}

	int NumAllocated() const {
		return m_nAllocationCount;
	}

	bool IsExternallyAllocated() const {
		return m_nGrowSize < 0;
	}

protected:
	T * m_pMemory;
	int m_nAllocationCount;
	int m_nGrowSize;
};

template <class T>
inline T* Construct( T* pMemory ) {
	return ::new(pMemory) T;
}

template <class T>
inline void Destruct( T* pMemory ) {
	pMemory->~T();
}

template< class T, class A = CUtlMemory<T> >
class CUtlVector {
	typedef A CAllocator;
public:
	T & operator[]( int i ) {
		return m_Memory[i];
	}

	T& Element( int i ) {
		return m_Memory[i];
	}

	T* Base() {
		return m_Memory.Base();
	}

	int Count() const {
		return m_Size;
	}

	void RemoveAll() {
		for (int i = m_Size; --i >= 0; )
			Destruct( &Element( i ) );

		m_Size = 0;
	}

	int AddToTail() {
		return InsertBefore( m_Size );
	}

	void SetSize( int size ) {
		m_Size = size;
	}

	int InsertBefore( int elem ) {
		GrowVector();
		ShiftElementsRight( elem );
		Construct( &Element( elem ) );

		return elem;
	}

protected:
	void GrowVector( int num = 1 ) {
		if (m_Size + num > m_Memory.NumAllocated())
			m_Memory.Grow( m_Size + num - m_Memory.NumAllocated() );

		m_Size += num;
		ResetDbgInfo();
	}

	void ShiftElementsRight( int elem, int num = 1 ) {
		int numToMove = m_Size - elem - num;
		if ((numToMove > 0) && (num > 0))
			memmove( &Element( elem + num ), &Element( elem ), numToMove * sizeof( T ) );
	}

	CAllocator m_Memory;
	int m_Size;

	T* m_pElements;

	inline void ResetDbgInfo() {
		m_pElements = Base();
	}
};

enum cvar_flags {
	fcvar_none = 0,
	fcvar_unregistered = (1 << 0),
	fcvar_developmentonly = (1 << 1),
	fcvar_gamedll = (1 << 2),
	fcvar_clientdll = (1 << 3),
	fcvar_hidden = (1 << 4),
	fcvar_protected = (1 << 5),
	fcvar_sponly = (1 << 6),
	fcvar_archive = (1 << 7),
	fcvar_notify = (1 << 8),
	fcvar_userinfo = (1 << 9),
	fcvar_printableonly = (1 << 10),
	fcvar_unlogged = (1 << 11),
	fcvar_never_as_string = (1 << 12),
	fcvar_replicated = (1 << 13),
	fcvar_cheat = (1 << 14),
	fcvar_ss = (1 << 15),
	fcvar_demo = (1 << 16),
	fcvar_dontrecord = (1 << 17),
	fcvar_ss_added = (1 << 18),
	fcvar_release = (1 << 19),
	fcvar_reload_materials = (1 << 20),
	fcvar_reload_textures = (1 << 21),
	fcvar_not_connected = (1 << 22),
	fcvar_material_system_thread = (1 << 23),
	fcvar_archive_xbox = (1 << 24),
	fcvar_accessible_from_threads = (1 << 25),
	fcvar_server_can_execute = (1 << 28),
	fcvar_server_cannot_query = (1 << 29),
	fcvar_clientcmd_can_execute = (1 << 30),
	fcvar_meme_dll = (1 << 31),
	fcvar_material_thread_mask = (fcvar_reload_materials | fcvar_reload_textures | fcvar_material_system_thread)
};

class ConVar
{
public:
	Color GetColor()
	{
		return Utils::CallVFunc<10, Color>( this );
	}

	const char* GetString()
	{
		return Utils::CallVFunc<11, const char*>( this );
	}

	float GetFloat()
	{
		return Utils::CallVFunc<12, float>( this );
	}

	int GetInt()
	{
		return Utils::CallVFunc<13, int>( this );
	}

	void SetValue( const char *value )
	{
		Utils::CallVFunc<14, void>( this, value );
	}

	void SetValue( float value )
	{
		Utils::CallVFunc<15, void>( this, value );
	}

	void SetValue( int value )
	{
		Utils::CallVFunc<16, void>( this, value );
	}

	void SetValue( Color value )
	{
		Utils::CallVFunc<17, void>( this, value );
	}

	bool GetBool()
	{
		return !!GetInt();
	}

	char pad_0x0000[0x4]; //0x0000
	ConVar *pNext; //0x0004
	int32_t bRegistered; //0x0008
	char *pszName; //0x000C
	char *pszHelpString; //0x0010
	int32_t nFlags; //0x0014
	char pad_0x0018[0x4]; //0x0018
	ConVar *pParent; //0x001C
	char *pszDefaultValue; //0x0020
	char *strString; //0x0024
	int32_t StringLength; //0x0028
	float fValue; //0x002C
	int32_t nValue; //0x0030
	int32_t bHasMin; //0x0034
	float fMinVal; //0x0038
	int32_t bHasMax; //0x003C
	float fMaxVal; //0x0040
	void *fnChangeCallback; //0x0044

public:
	convar * parent;
	char* default_value;
	char* string;
	__int32 string_length;
	float float_value;
	__int32 numerical_value;
	__int32 has_min;
	float min;
	__int32 has_max;
	float max;
	CUtlVector<fn_change_callback_t> callbacks;
};

#define PRINTF_FORMAT_STRING _Printf_format_string_
class ICVar {
public:
	virtual void Func0();
	virtual void Func1();
	virtual void Func2();
	virtual void Func3();
	virtual void Func4();
	virtual void Func5();
	virtual void Func6();
	virtual void Func7();
	virtual void Func8();
	virtual void Func9();
	virtual void RegisterConCommand( ConVar *pCommandBase ) = 0;
	virtual void UnregisterConCommand( ConVar *pCommandBase ) = 0;
	virtual void Func12();
	virtual void Func13();
	virtual void Func14();
	virtual void Func15();
	virtual ConVar* FindVar( const char* getVar );
	virtual void Func17();
	virtual void Func18();
	virtual void Func19();
	virtual void Func20();
	virtual void Func21();
	virtual void Func22();
	virtual void Func23();
	virtual void Func24();
	virtual void ConsoleColorPrintf( const Color& clr, const char *format, ... );
	virtual void ConsolePrintf( const char *format, ... );
	virtual void ConsoleDPrintf( PRINTF_FORMAT_STRING const char *pFormat, ... );
	virtual void Func28();
	virtual void Func29();
	virtual void Func30();
	virtual void Func31();
	virtual void Func32();
	virtual void Func33();
	virtual void Func34();
	virtual void Func35();
	virtual void Func36();
	virtual void Func37();
};

extern ICVar* g_pCVar;