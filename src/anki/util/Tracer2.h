// Copyright (C) 2009-2019, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/util/Thread.h>
#include <anki/util/WeakArray.h>
#include <anki/util/DynamicArray.h>
#include <anki/util/Singleton.h>
#include <anki/util/String.h>

namespace anki
{

/// @addtogroup util_other
/// @{

/// @memberof Tracer
class Tracer2EventHandle
{
	friend class Tracer2;

private:
	Second m_start;
};

/// @memberof Tracer
class Tracer2Event
{
public:
	CString m_name;
	Second m_start;
	Second m_duration;

	Tracer2Event()
	{
		// No init
	}
};

/// @memberof Tracer
class Tracer2Counter
{
public:
	CString m_name;
	U64 m_value;

	Tracer2Counter()
	{
		// No init
	}
};

/// Tracer flush callback.
/// @memberof Tracer
using Tracer2FlushCallback = void (*)(
	void* userData, ThreadId tid, ConstWeakArray<Tracer2Event> events, ConstWeakArray<Tracer2Counter> counters);

/// Tracer.
class Tracer2 : public NonCopyable
{
public:
	Tracer2(GenericMemoryPoolAllocator<U8> alloc)
		: m_alloc(alloc)
	{
	}

	~Tracer2();

	/// Begin a new event.
	/// @note It's thread-safe.
	ANKI_USE_RESULT Tracer2EventHandle beginEvent();

	/// End the event that got started with beginEvent().
	/// @note It's thread-safe.
	void endEvent(const char* eventName, Tracer2EventHandle event);

	/// Increment a counter.
	/// @note It's thread-safe.
	void incrementCounter(const char* counterName, U64 value);

	/// Flush all counters and events and start clean. The callback will be called multiple times.
	/// @note It's thread-safe.
	void flush(Tracer2FlushCallback callback, void* callbackUserData);

	Bool getEnabled() const
	{
		return m_enabled;
	}

	void setEnabled(Bool enabled)
	{
		m_enabled = enabled;
	}

private:
	static constexpr U32 EVENTS_PER_CHUNK = 256;
	static constexpr U32 COUNTERS_PER_CHUNK = 512;

	class ThreadLocal;
	class Chunk;

	GenericMemoryPoolAllocator<U8> m_alloc;

	static thread_local ThreadLocal* m_threadLocal;
	DynamicArray<ThreadLocal*> m_allThreadLocal; ///< The Tracer should know about all the ThreadLocal.
	Mutex m_allThreadLocalMtx;

	Bool m_enabled = false;

	/// Get the thread local ThreadLocal structure.
	/// @note Thread-safe.
	ThreadLocal& getThreadLocal();

	/// Get or create a new chunk.
	Chunk& getOrCreateChunk(ThreadLocal& tlocal);
};

/// The global tracer.
using Tracer2Singleton = SingletonInit<Tracer2>;

/// Scoped tracer event.
class Tracer2ScopedEvent
{
public:
	Tracer2ScopedEvent(const char* name)
		: m_name(name)
		, m_tracer(&Tracer2Singleton::get())
	{
		m_handle = m_tracer->beginEvent();
	}

	~Tracer2ScopedEvent()
	{
		m_tracer->endEvent(m_name, m_handle);
	}

private:
	const char* m_name;
	Tracer2EventHandle m_handle;
	Tracer2* m_tracer;
};

#if ANKI_ENABLE_TRACE
#	define ANKI_TRACE2_SCOPED_EVENT(name_) Tracer2ScopedEvent _tse##name_(#	name_)
#	define ANKI_TRACE2_INC_COUNTER(name_, val_) Tracer2Singleton::get().increaseCounter(#	name_, val_)
#else
#	define ANKI_TRACE2_SCOPED_EVENT(name_) ((void)0)
#	define ANKI_TRACE2_INC_COUNTER(name_) ((void)0)
#endif
/// @}

} // end namespace anki
