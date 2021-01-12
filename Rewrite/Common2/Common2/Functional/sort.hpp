#pragma once

struct Sort
{
	struct Quick
	{

	};

	template<class ElementType>
	struct Linear
	{
		Slice<ElementType> slice;
		size_t current_index;
		size_t pivot_index;

		struct In {
			char current_comparison_result;
		} in;

		struct Out
		{
			ElementType* left;
			ElementType* right;
		} out;

		enum class State
		{
			INCREMENT_PIVOT = 0,
			SORT_LOOP = 1,
			ENDED = 2
		} state;

		inline static Linear<ElementType> build(const Slice<ElementType>* p_slice, const size_t p_start_index)
		{
			Linear<ElementType> l_linear;
			l_linear.slice = *p_slice;
			l_linear.pivot_index = p_start_index - 1;
			l_linear.state = State::INCREMENT_PIVOT;
			return l_linear;
		};

		inline static Linear<ElementType> build_start_0(const Slice<ElementType>* p_slice)
		{
			Linear<ElementType> l_linear;
			l_linear.slice = *p_slice;
			l_linear.pivot_index = -1;
			l_linear.state = State::INCREMENT_PIVOT;
			return l_linear;
		};

		inline static Linear<ElementType> build_start_0(const Slice<ElementType> p_slice)
		{
			return build_start_0(&p_slice);
		};

		inline char step()
		{
			switch (this->state)
			{
			case State::INCREMENT_PIVOT:
			{
				this->pivot_index += 1;

				if (this->pivot_index == this->slice.Size - 1)
				{
					this->state = State::ENDED;
					return false;
				}

				this->current_index = this->pivot_index + 1;
				this->state = State::SORT_LOOP;
				this->out = Out{ this->slice.get(this->pivot_index), this->slice.get(this->current_index) };
				return true;
			};
			break;
			case State::SORT_LOOP:
			{
				if (this->in.current_comparison_result)
				{
					ElementType l_left_tmp = *this->out.left;
					*this->out.left = *this->out.right;
					*this->out.right = l_left_tmp;
				}

				this->current_index += 1;
				if (this->current_index == this->slice.Size)
				{
					this->state = State::INCREMENT_PIVOT;
				}
				else
				{
					this->out.right = this->slice.get(this->current_index);
				}
				return true;
			}
			break;
			}

			return false;
		};
	};
};