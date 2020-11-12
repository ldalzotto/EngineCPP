#pragma once

template<class ElementType>
struct Mutex
{
public:
	enum State
	{
		FREE = 0,
		LOCKED = 1
	} state = State::FREE;

	Mutex() = default;

	inline Mutex(ElementType& p_resource) 
	{
		this->resource = &p_resource;
	};

	inline ElementType& get()
	{
		while (this->state == State::LOCKED) {}
		this->state = State::LOCKED;
		return *this->resource;
	};

	inline void give()
	{
		this->state = State::FREE;
	};

private:
	ElementType* resource;
};