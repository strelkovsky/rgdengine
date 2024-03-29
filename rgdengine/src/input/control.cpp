#include "precompiled.h"

#include <rgde/input/input.h>
#include "inputimpl.h"

namespace input
{
    Control::Control(controls name, type_t tipe, device &device)
		: m_device(device)
        , m_type(tipe)
        , m_time(0)
        , m_press(false)
        , m_delta(0)
        , m_name(name)
    {
    }

    void Control::bind (command_ptr pCommand)
    {
        commands_iter pos = std::find
        (
          m_commands.begin(),
          m_commands.end(),
          pCommand
        );

        if (pos != m_commands.end())
            return;

        m_commands.push_back(pCommand);
    }

    void Control::bind (const std::string &command_name)
    {
        bind (m_device.get_impl().get_command(command_name));
    }

    // remove observer
    void Control::unbind (command_ptr pCommand)
    {
        commands_iter pos = std::find
        (
          m_commands.begin(),
          m_commands.end(),
          pCommand
        );

        if (pos == m_commands.end())
            return;

        m_commands.erase(pos);
    }

    void Control::unbind (const std::string &command_name)
    {
        unbind(m_device.get_impl().get_command(command_name));
    }

    // check is that observer already added
    bool Control::is_bind (command_ptr pCommand)
    {
        commands_iter pos = std::find
        (
          m_commands.begin(),
          m_commands.end(),
          pCommand
        );

        if (pos == m_commands.end())
            return false;

        return true;
    }

    bool Control::is_bind (const std::string &command_name)
    {
        return is_bind(m_device.get_impl().get_command(command_name));
    }

    // notify observers about control state change
    void Control::notify_all ()
    {
		if (m_commands.empty())
			return;

        commands_iter i = m_commands.begin();

        while (i != m_commands.end())
        {
			command_ptr command = *i;
			if (command)
			{
				command->notify_all(*this);
			}
            ++i;
        }
    }

}