#pragma once

#include <atomic>

class RefCounted
{
public:
	void IncRefCount() const
	{
		m_RefCount++;
	}

	void DecRefCount() const
	{
		m_RefCount--;
	}

	uint32_t GetRefCount() const { return m_RefCount; }
private:
	mutable std::atomic<uint32_t> m_RefCount = 0;
};

template<typename T>
class Ref
{
public:
	Ref()
		: m_Instance(nullptr)
	{
	}

	Ref(std::nullptr_t n)
		: m_Instance(nullptr)
	{
	}

	Ref(T* instance)
		: m_Instance(instance)
	{
		static_assert(std::is_base_of<RefCounted, T>::value, "Class is not RefCounted!");
		IncRef();
	}

	Ref(const Ref<T>& other)
		: m_Instance(other.m_Instance)
	{
		IncRef();
	}

	~Ref()
	{
		DecRef();
	}

	Ref& operator=(const Ref<T>& other)
	{
		if (this != &other)
		{
			DecRef();
			m_Instance = other.m_Instance;
			IncRef();
		}
		return *this;
	}

	Ref& operator=(Ref<T>&& other)
	{
		DecRef();

		m_Instance = other.m_Instance;
		other.m_Instance = nullptr;
		return *this;
	}

	operator bool() const { return m_Instance != nullptr; }
	
	bool operator==(const Ref<T>& other) const
	{
		return m_Instance == other.m_Instance;
	}

	bool operator!=(const Ref<T>& other) const
	{
		return !(*this == other);
	}

	T* operator->() { return m_Instance; }
	const T* operator->() const { return m_Instance; }

	T& operator*() { return *m_Instance; }
	const T& operator*() const { return *m_Instance; }

	T* Raw() { return m_Instance; }
	const T* Raw() const { return m_Instance; }

	template<typename... Args>
	static Ref<T> Create(Args&&... args)
	{
		return Ref<T>(new T(std::forward<Args>(args)...));
	}
private:
	void IncRef() const
	{
		if (m_Instance)
			m_Instance->IncRefCount();
	}

	void DecRef() const
	{
		if (m_Instance)
		{
			m_Instance->DecRefCount();
			if (m_Instance->GetRefCount() == 0)
			{
				delete m_Instance;
				m_Instance = nullptr;
			}
		}
	}
private:
	mutable T* m_Instance;
};