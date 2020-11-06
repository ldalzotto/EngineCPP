#pragma once

typedef void* RenderHandle;

RenderHandle create_render();
void destroy_render(const RenderHandle& p_render);
bool render_window_should_close(const RenderHandle& p_render);
void render_window_pool_event(const RenderHandle& p_render);
void render_draw(const RenderHandle& p_render);