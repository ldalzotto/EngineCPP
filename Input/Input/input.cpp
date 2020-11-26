
#include "Input/input.hpp"
#include "Common/Container/vector.hpp"
#include "Common/Container/pool.hpp"
#include "GLFW/glfwinclude.h"

enum class InputEventType
{
	PRESSED = 0,
	RELEASED = 1,
	REPEAT = 2
};

struct InputEvent
{
	InputKey key;
	InputEventType type;
};

struct GlobalInputEventHandler
{
	void* window;
	com::Vector<InputEvent> events;
};

com::Vector<GlobalInputEventHandler> GlobalInputEventHandlers;

void handle_global_input_event(GLFWwindow* p_window, int key, int scancode, int action, int mods);

struct Input
{
	void* window;

	com::Vector<KeyState> InputState;

	com::Vector<InputEvent> InputEventsLastFrame;
	com::Vector<InputKey> InputKeysReleasedThisFrame;
	com::Vector<InputKey> InputKeysJustPressedThisFrame;

	inline void allocate(void* p_window)
	{
		this->window = p_window;

		glfwSetKeyCallback((GLFWwindow*)window, handle_global_input_event);

		this->InputState.allocate((size_t)InputKey::InputKey_LAST);
		this->InputState.Size = this->InputState.Capacity;
		memset(this->InputState.Memory, 0, this->InputState.size_in_bytes());

		GlobalInputEventHandler l_global_handler;
		l_global_handler.window = this->window;
		GlobalInputEventHandlers.push_back(l_global_handler);

	};

	inline void free()
	{
		for (short int i = 0; i < GlobalInputEventHandlers.Size; i++)
		{
			if (GlobalInputEventHandlers[i].window == this->window)
			{
				GlobalInputEventHandlers[i].events.free();
				break;
			}
		}

		this->InputState.free();
		this->InputKeysReleasedThisFrame.free();
		this->InputKeysJustPressedThisFrame.free();
		this->InputEventsLastFrame.free();
	};

	inline void consume_input_events()
	{
		GlobalInputEventHandler* l_handler = nullptr;
		for (short int i = 0; i < GlobalInputEventHandlers.Size; i++)
		{
			if (GlobalInputEventHandlers[i].window == this->window)
			{
				l_handler = &GlobalInputEventHandlers[i];
				break;
			}
		}

		if (l_handler)
		{
			for (size_t i = 0; i < l_handler->events.Size; i++)
			{
				this->handle_event(l_handler->events[i]);
			}
			l_handler->events.clear();

			for (size_t i = 0; i < this->InputEventsLastFrame.Size; i++)
			{
				this->handle_event(this->InputEventsLastFrame[i]);
			}
			this->InputEventsLastFrame.clear();
		}
	};

	inline void handle_event(const InputEvent& p_inputEvent)
	{
		KeyState& l_oldStateFlag = this->InputState.Memory[(size_t)p_inputEvent.key];

		if (((l_oldStateFlag == KeyState::KeyStateFlag_PRESSED) || (l_oldStateFlag == KeyState::KeyStateFlag_PRESSED_THIS_FRAME)) && p_inputEvent.type == InputEventType::RELEASED)
		{
			l_oldStateFlag = KeyState::KeyStateFlag_RELEASED_THIS_FRAME;
			this->InputKeysReleasedThisFrame.push_back(p_inputEvent.key);
		}
		else if ((l_oldStateFlag == KeyState::KeyStateFlag_PRESSED_THIS_FRAME) && (p_inputEvent.type == InputEventType::REPEAT || p_inputEvent.type == InputEventType::PRESSED))
		{
			l_oldStateFlag = KeyState::KeyStateFlag_PRESSED;
		}
		else if (((l_oldStateFlag == KeyState::KeyStateFlag_NONE) || (l_oldStateFlag == KeyState::KeyStateFlag_RELEASED_THIS_FRAME)) && p_inputEvent.type == InputEventType::PRESSED)
		{
			l_oldStateFlag = KeyState::KeyStateFlag_PRESSED_THIS_FRAME;
			this->InputKeysJustPressedThisFrame.push_back(p_inputEvent.key);
		}
	};

	inline void new_frame()
	{
		if (this->InputKeysReleasedThisFrame.Size > 0)
		{
			for (size_t i = 0; i < this->InputKeysReleasedThisFrame.Size; i++)
			{
				this->InputState[(size_t)this->InputKeysReleasedThisFrame[i]] = KeyState::KeyStateFlag_NONE;
			}

			this->InputKeysReleasedThisFrame.clear();
		}

		for (size_t i = 0; i < this->InputKeysJustPressedThisFrame.Size; i++)
		{
			InputKey l_inputKeyCode = this->InputKeysJustPressedThisFrame.Memory[i];

			InputEvent l_event;
			l_event.key = l_inputKeyCode;
			l_event.type = InputEventType::REPEAT;
			this->InputEventsLastFrame.push_back(l_event);
		}
		this->InputKeysJustPressedThisFrame.clear();

		this->consume_input_events();
	};

	inline bool get_state(InputKey p_key, KeyState p_state)
	{
		return (short int)this->InputState[(size_t)p_key] & (short int)p_state;
	};
};

void handle_global_input_event(GLFWwindow* p_window, int key, int scancode, int action, int mods)
{
	GlobalInputEventHandler* l_handler;
	for (short int i = 0; i < GlobalInputEventHandlers.Size; i++)
	{
		if (GlobalInputEventHandlers[i].window == p_window)
		{
			l_handler = &GlobalInputEventHandlers[i];
			break;
		}
	}

	InputEvent l_event;
	switch (key)
	{
	case GLFW_KEY_A: l_event.key = InputKey::InputKey_A; break;
	case GLFW_KEY_B: l_event.key = InputKey::InputKey_B; break;
	case GLFW_KEY_C: l_event.key = InputKey::InputKey_C; break;
	case GLFW_KEY_D: l_event.key = InputKey::InputKey_D; break;
	case GLFW_KEY_E: l_event.key = InputKey::InputKey_E; break;
	case GLFW_KEY_F: l_event.key = InputKey::InputKey_F; break;
	case GLFW_KEY_G: l_event.key = InputKey::InputKey_G; break;
	case GLFW_KEY_H: l_event.key = InputKey::InputKey_H; break;
	case GLFW_KEY_I: l_event.key = InputKey::InputKey_I; break;
	case GLFW_KEY_J: l_event.key = InputKey::InputKey_J; break;
	case GLFW_KEY_K: l_event.key = InputKey::InputKey_K; break;
	case GLFW_KEY_L: l_event.key = InputKey::InputKey_L; break;
	case GLFW_KEY_M: l_event.key = InputKey::InputKey_M; break;
	case GLFW_KEY_N: l_event.key = InputKey::InputKey_N; break;
	case GLFW_KEY_O: l_event.key = InputKey::InputKey_O; break;
	case GLFW_KEY_P: l_event.key = InputKey::InputKey_P; break;
	case GLFW_KEY_Q: l_event.key = InputKey::InputKey_Q; break;
	case GLFW_KEY_R: l_event.key = InputKey::InputKey_R; break;
	case GLFW_KEY_S: l_event.key = InputKey::InputKey_S; break;
	case GLFW_KEY_T: l_event.key = InputKey::InputKey_T; break;
	case GLFW_KEY_U: l_event.key = InputKey::InputKey_U; break;
	case GLFW_KEY_V: l_event.key = InputKey::InputKey_V; break;
	case GLFW_KEY_W: l_event.key = InputKey::InputKey_W; break;
	case GLFW_KEY_X: l_event.key = InputKey::InputKey_X; break;
	case GLFW_KEY_Y: l_event.key = InputKey::InputKey_Y; break;
	case GLFW_KEY_Z: l_event.key = InputKey::InputKey_Z; break;
	case GLFW_KEY_RIGHT: l_event.key = InputKey::InputKey_RIGHT; break;
	case GLFW_KEY_LEFT: l_event.key = InputKey::InputKey_LEFT;	 break;
	case GLFW_KEY_DOWN: l_event.key = InputKey::InputKey_DOWN;	 break;
	case GLFW_KEY_UP: l_event.key = InputKey::InputKey_UP;		 break;
	default: return;
	}

	switch (action)
	{
	case GLFW_PRESS: l_event.type = InputEventType::PRESSED;    break;
	case GLFW_RELEASE: l_event.type = InputEventType::RELEASED;	break;
	case GLFW_REPEAT: l_event.type = InputEventType::REPEAT;	break;
	default: return;
	}

	l_handler->events.push_back(l_event);
}


void InputHandle::allocate(void* p_window)
{
	Input* l_input = new Input();
	l_input->allocate(p_window);
	this->handle = l_input;
};

void InputHandle::free()
{
	((Input*)this->handle)->free();
	delete (Input*)this->handle;
};

void InputHandle::new_frame()
{
	((Input*)this->handle)->new_frame();
};

bool InputHandle::get_state(InputKey p_key, KeyState p_state)
{
	return ((Input*)this->handle)->get_state(p_key, p_state);
};

