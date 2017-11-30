#ifndef WINDOW_XCB_H_
#define WINDOW_XCB_H_

#include "window.h"
#include "Input/input_manager.h"
#include <assert.h>
#include <string>
#include <iostream>

namespace ENGINE
{


#if VK_USE_PLATFORM_XCB_KHR
	

	void Window::initOSWindow()
	{
		// create connection to X11 server
		const xcb_setup_t		*	setup = nullptr;
		xcb_screen_iterator_t		iter;
		int							screen = 0;

		xcbConnection = xcb_connect(nullptr, &screen);
		if (xcbConnection == nullptr) {
			std::cout << "Cannot find a compatible Vulkan ICD.\n";
			exit(-1);
		}

		setup = xcb_get_setup(xcbConnection);
		iter = xcb_setup_roots_iterator(setup);
		while (screen-- > 0) {
			xcb_screen_next(&iter);
		}
		xcbScreen = iter.data;

		// create window
		VkRect2D dimensions = { { 0, 0 },{ _surface_size_x, _surface_size_y } };

		assert(dimensions.extent.width > 0);
		assert(dimensions.extent.height > 0);

		uint32_t value_mask, value_list[32];

		xcbWindow = xcb_generate_id(xcbConnection);

		value_mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
		value_list[0] = xcbScreen->black_pixel;
		value_list[1] = XCB_EVENT_MASK_KEY_RELEASE | XCB_EVENT_MASK_EXPOSURE;

		xcb_create_window(xcbConnection, XCB_COPY_FROM_PARENT, xcbWindow,
			xcbScreen->root, dimensions.offset.x, dimensions.offset.y,
			dimensions.extent.width, dimensions.extent.height, 0,
			XCB_WINDOW_CLASS_INPUT_OUTPUT, xcbScreen->root_visual,
			value_mask, value_list);

		/* Magic code that will send notification when window is destroyed */
		xcb_intern_atom_cookie_t cookie =
			xcb_intern_atom(xcbConnection, 1, 12, "WM_PROTOCOLS");
		xcb_intern_atom_reply_t *reply =
			xcb_intern_atom_reply(xcbConnection, cookie, 0);

		xcb_intern_atom_cookie_t cookie2 =
			xcb_intern_atom(xcbConnection, 0, 16, "WM_DELETE_WINDOW");
		_xcb_atom_window_reply =
			xcb_intern_atom_reply(xcbConnection, cookie2, 0);

		xcb_change_property(xcbConnection, XCB_PROP_MODE_REPLACE, xcbWindow,
			(*reply).atom, 4, 32, 1,
			&(*_xcb_atom_window_reply).atom);
		free(reply);

		xcb_map_window(xcbConnection, xcbWindow);

		// Force the x/y coordinates to 100,100 results are identical in consecutive
		// runs
		const uint32_t coords[] = { 100, 100 };
		xcb_configure_window(xcbConnection, xcbWindow,
			XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y, coords);
		xcb_flush(xcbConnection);
	}


	void Window::deInitOSWindow()
	{
		xcb_destroy_window(xcbConnection, xcbWindow);
		xcb_disconnect(xcbConnection);
		xcbWindow = 0;
		xcbConnection = nullptr;
	}

	void Window::updateOSWindow()
	{
		auto event = xcb_poll_for_event(xcbConnection);

		// if there is no event, event will be NULL
		// need to check for event == NULL to prevent segfault
		if (!event)
			return;

		switch (event->response_type & ~0x80) {
		case XCB_CLIENT_MESSAGE:
			if (((xcb_client_message_event_t*)event)->data.data32[0] == xcbAtomWindowReply->atom) {
				Close();
			}
			break;
		default:
			break;
}
		free(event);
	}

	void Window::initOSSurface(const VkInstance& instance)
	{
		VkXcbSurfaceCreateInfoKHR createInfo{};
		createInfo.sType	  = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
		createInfo.connection = xcbConnection;
		createInfo.window     = xcbWindow;
		vkCreateXcbSurfaceKHR(instance, &createInfo, nullptr, &surface);
	}

#endif

}

#endif // WINDOW_XCB_H_
