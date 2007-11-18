#pragma once

#include <list>

namespace rgde
{
	namespace render
	{
		class device;

		class device_object
		{
		public:
			explicit device_object(device& dev, bool register_on_lost_reset = false);
			virtual ~device_object();

			device& get_device() {return m_device;}
			const device& get_device() const {return m_device;}

			static size_t get_objects_num();

			virtual void on_device_lost() {}
			virtual void on_device_reset() {}

		private:
			typedef std::list<device_object*>::iterator obj_handler;

			device& m_device;
			bool m_is_registered;
			obj_handler m_handler;
		};
	}
}