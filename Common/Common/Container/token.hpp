#pragma once

struct Token
{
	size_t val;
};

inline Token Token_build()
{
	return Token{ (size_t)-1 };
};

inline Token Token_build(const size_t p_val)
{
	return Token{ p_val };
};

inline void Token_reset(Token* p_token)
{
	*p_token = Token_build();
};

template<class ElementType>
struct TToken
{
	union
	{
		Token token;
		size_t val;
	};
};

template<class ElementType>
inline TToken<ElementType> TToken_build()
{
	return TToken<ElementType> { Token_build() };
};

template<class ElementType>
inline TToken<ElementType> TToken_build(const size_t p_val)
{
	return TToken<ElementType> { Token_build(p_val) };
};

template<class ElementType, class OtherType>
inline TToken<OtherType>* TToken_cast(TToken<ElementType>* p_token)
{
	return (TToken<OtherType>*)p_token;
};

template<class ElementType>
inline TToken<ElementType>* TToken_cast_numeric(size_t* p_token)
{
	return (TToken<ElementType>*)p_token;
};

template<class ElementType, class OtherType>
inline bool TToken_equals(TToken<ElementType>* p_token, TToken<OtherType>* p_other)
{
	return p_token->val == p_other->val;
};

