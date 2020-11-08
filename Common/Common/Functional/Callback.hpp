#pragma once

template<class Closure, class Parameter>
struct Callback
{
	Closure* closure;
	void(*fn)(Closure* p_closure, Parameter* p_parameter);

	inline Callback() {
		this->closure = nullptr;
		this->fn = nullptr;
	};

	inline Callback(Closure* p_closure, void(*p_fn)(Closure* p_closure, Parameter* p_parameter))
	{
		this->closure = p_closure;
		this->fn = p_fn;
	};

	inline void call(Parameter* p_parameter)
	{
		this->fn(this->closure, p_parameter);
	}
};