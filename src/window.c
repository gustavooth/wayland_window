#include "defines.h"

#include <stdio.h>
#include <wayland-client.h>
#include "xdg-shell-client-protocol.h"
#include <string.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdlib.h>

#define WIDTH 1280
#define HEIGHT 720

typedef struct WaylandState {
  struct wl_display* display;
  struct wl_registry* registry;
  struct wl_compositor* compositor;
  struct wl_surface* surface;

  struct wl_shm* shm;
  struct wl_buffer* buffer;
  u8* pixels;

  struct xdg_wm_base* wm_base;
  struct xdg_surface* xsurface;
  struct xdg_toplevel* top;
} WaylandState;

void draw(WaylandState* state);

void registry_listener_global(
  void* data, struct wl_registry* wl_registry, u32 name, const char* interface, u32 version);

void registry_listener_remove(
  void *data, struct wl_registry *wl_registry, u32 name);

const struct wl_registry_listener registry_listener = {
  .global = registry_listener_global,
  .global_remove = registry_listener_remove
};

void xsurface_configure(void* data, struct xdg_surface* xsurface, u32 serial);

const struct xdg_surface_listener xsurface_listener = {
  .configure = xsurface_configure
};

void wm_base_ping(void* data, struct xdg_wm_base* wm_base, u32 serial);

const struct xdg_wm_base_listener wm_base_listener = {
  .ping = wm_base_ping
};

void top_configure(void* data, struct xdg_toplevel* top, i32 width, i32 height, struct wl_array *states);
void top_close(void* data, struct xdg_toplevel* top);
void top_configure_bounds(void* data, struct xdg_toplevel* top, i32 width, i32 height);
void top_wm_capabilities(void* data, struct xdg_toplevel* top, struct wl_array* capabilities);

const struct xdg_toplevel_listener top_listener = {
  .configure = top_configure,
  .close = top_close,
  .configure_bounds = top_configure_bounds,
  .wm_capabilities = top_wm_capabilities
};

i32 alc_shm(u64 sz) {
	int8_t name[8];
	name[0] = '/';
	name[7] = 0;
	for (uint8_t i = 1; i < 6; i++) {
		name[i] = (rand() & 23) + 97;
	}

	i32 fd = shm_open(name, O_RDWR | O_CREAT | O_EXCL, S_IWUSR | S_IRUSR | S_IWOTH | S_IROTH);
	shm_unlink(name);
	ftruncate(fd, sz);

	return fd;
}

void resize(WaylandState* state) {
	i32 fd = alc_shm(WIDTH * HEIGHT * 4);

	state->pixels = mmap(0, WIDTH * HEIGHT * 4, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	struct wl_shm_pool* pool = wl_shm_create_pool(state->shm, fd, WIDTH * HEIGHT * 4);
	state->buffer = wl_shm_pool_create_buffer(pool, 0, WIDTH, HEIGHT, WIDTH * 4, WL_SHM_FORMAT_ARGB8888);
	wl_shm_pool_destroy(pool);
	close(fd);
}

int main() {
  WaylandState state = {0};
  state.display = wl_display_connect(0);
  state.registry = wl_display_get_registry(state.display);
  wl_registry_add_listener(state.registry, &registry_listener, &state);

  wl_display_roundtrip(state.display);

  xdg_wm_base_add_listener(state.wm_base, &wm_base_listener, &state);

  state.surface = wl_compositor_create_surface(state.compositor);

  state.xsurface = xdg_wm_base_get_xdg_surface(state.wm_base, state.surface);
  xdg_surface_add_listener(state.xsurface, &xsurface_listener, &state);
  state.top = xdg_surface_get_toplevel(state.xsurface);
  xdg_toplevel_set_title(state.top, "Wayland client");
  wl_surface_commit(state.surface);

  while (wl_display_dispatch(state.display))
  {
    /* code */
  }
  

  wl_surface_destroy(state.surface);
  wl_display_disconnect(state.display);
  return 0;
}

void registry_listener_global(void* data, struct wl_registry* wl_registry, u32 name, const char* interface, u32 version) {
  WaylandState* state = (WaylandState*)data;
    
  if(!strcmp(interface, wl_compositor_interface.name)) {
    state->compositor = wl_registry_bind(wl_registry, name, &wl_compositor_interface, version);
  }else if(!strcmp(interface, wl_shm_interface.name)) {
    state->shm = wl_registry_bind(wl_registry, name, &wl_shm_interface, version);
  }else if(!strcmp(interface, xdg_wm_base_interface.name)) {
    state->wm_base = wl_registry_bind(wl_registry, name, &xdg_wm_base_interface, version);
  }
}

void registry_listener_remove(void *data, struct wl_registry *wl_registry, uint32_t name) {
  WaylandState* state = (WaylandState*)data;
}

void draw(WaylandState* state) {

}

void xsurface_configure(void* data, struct xdg_surface* xsurface, u32 serial) {
  WaylandState* state = (WaylandState*)data;

  xdg_surface_ack_configure(xsurface, serial);

  if(!state->pixels)
    resize(state);

  draw(state);
  wl_surface_attach(state->surface, state->buffer, 0, 0);
  wl_surface_damage(state->surface, 0, 0, WIDTH, HEIGHT);
  wl_surface_commit(state->surface);
}

void wm_base_ping(void* data, struct xdg_wm_base* wm_base, u32 serial) {
  WaylandState* state = (WaylandState*)data;

  xdg_wm_base_pong(wm_base, serial);
}

void top_configure(void* data, struct xdg_toplevel* top, i32 width, i32 height, struct wl_array *states) {}
void top_close(void* data, struct xdg_toplevel* top) {}
void top_configure_bounds(void* data, struct xdg_toplevel* top, i32 width, i32 height) {}
void top_wm_capabilities(void* data, struct xdg_toplevel* top, struct wl_array* capabilities) {}