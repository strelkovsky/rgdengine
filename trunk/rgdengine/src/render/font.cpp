#include "precompiled.h"

#include <rgde/render/font.h>
#include <rgde/render/manager.h>
#include <rgde/render/device.h>

#include <d3dx9.h>
extern LPDIRECT3DDEVICE9 g_d3d;

namespace render
{
	class font_manager : public rendererable
	{
	protected:
		font_manager() : rendererable(10002)
		{
			m_renderInfo.render_func = boost::bind(&font_manager::renderAll, this);
		}

	public:
		typedef boost::function<void(void)>	FontRenderCallback;
		typedef std::list<FontRenderCallback>	CBList;

		CBList texts;		

		void addText(FontRenderCallback cb)
		{
			// ��� � ������ �� ����� ������, ���� � ��� �� ����� �������.
			// ���� ������ ��������.
			texts.push_back(cb);
		}		

		void renderAll()
		{
			for (CBList::iterator it = texts.begin(); it != texts.end(); ++it)
				(*it)();
			texts.clear();
		}		

		renderable_info & getRenderableInfo()
		{
			return m_renderInfo;
		}
	};	

	typedef base::singelton<font_manager> TheFontRenderManager;

	static int fontsCreated	= 0;

	void base_font::render(const std::wstring &text, const math::Rect &rect, unsigned int color)
	{
		render(text, rect, color, false, Top | Left | WordBreak);
	}	

	void base_font::render(const std::wstring &text, const math::Rect &rect, unsigned int color, bool isDrawShadow)
	{
		render(text, rect, color, isDrawShadow, Top | Left | WordBreak);
	}	

	class FontImpl : public base_font, public device_object
	{
		int				m_nHeight;
		std::wstring	m_name;
		FontWeight		m_eFontWeght;
		bool			m_useDelayedRender;

		void disableDelayedRender(bool b)
		{
			m_useDelayedRender = !b;
		}

		void destroy()
		{
			if (m_pFont != NULL)
			{
				m_pFont->Release();
				m_pFont = 0;
				fontsCreated--;
			}
		}

		void create()
		{
			if (g_d3d == NULL || m_pFont != NULL)
				return;

			if (FAILED(D3DXCreateFont(g_d3d, -m_nHeight, 0, m_eFontWeght, 1, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, 5, DEFAULT_PITCH | FF_DONTCARE, m_name.c_str(), &m_pFont)))
			{
				throw std::bad_exception("FontImpl():Can't create device font object!");
			}			fontsCreated++;
		}

	public:
		FontImpl(int height, const std::wstring &name, FontWeight font_weigh)
			: m_pFont(0),
			  m_nHeight(height),
			  m_name(name),
			  m_eFontWeght(font_weigh),
			  m_useDelayedRender(true)
		{
			base::lmsg << "Creating Font:  \"" << std::string(name.begin(), name.end()) << "\"," << m_nHeight;
			create();
		}		

		void doRender(const std::wstring text, RECT textLocation, unsigned int color, int flags)
		{
			if (m_pFont != NULL)
				m_pFont->DrawText(NULL, text.c_str(), -1, &textLocation, flags, color);
		}		

		math::Rect get_rect(const std::wstring &text, int flags)
		{
			RECT rc	= {0, 0, 0, 200};

			if (m_pFont != NULL)
				m_pFont->DrawText(NULL, text.c_str(), -1, &rc, DT_CALCRECT | flags, 0);

			math::Rect	ret((float)rc.left, (float)rc.top, (float)rc.right - rc.left, (float)rc.bottom - rc.top);
			return ret;
		}		

		virtual void render(const std::wstring &text, const math::Rect &rect, unsigned int color, bool isDrawShadow, int flags)
		{
			const math::Vec2f virtSize(800, 600);

			unsigned	nShadowDistance	= 2;
			unsigned	nShadowColor	= 0xFF000000; //Black
			math::Vec2f	screen_size		= render::TheDevice::get().getBackBufferSize();
			math::Vec2f	ratio = screen_size / virtSize;

			font_manager& rm = TheFontRenderManager::get();

			if (isDrawShadow)
			{
				RECT	textShadowLocation;
				textShadowLocation.left = (LONG)((rect.position[0] + nShadowDistance) * ratio[0]);
				textShadowLocation.top = (LONG)((rect.position[1] + nShadowDistance) * ratio[1]);
				textShadowLocation.right = textShadowLocation.left + (LONG)(rect.size[0] * ratio[0]);
				textShadowLocation.bottom = textShadowLocation.top + (LONG)(rect.size[1] * ratio[1]);

				//doRender(text, textShadowLocation, nShadowColor, flags);
				rm.addText(boost::bind(&FontImpl::doRender, this, text, textShadowLocation, nShadowColor, flags));
			}

			RECT	textLocation;
			textLocation.left = (LONG)(rect.position[0] * ratio[0]);
			textLocation.top = (LONG)(rect.position[1] * ratio[1]);
			textLocation.right = textLocation.left + (LONG)(rect.size[0] * ratio[0]);
			textLocation.bottom = textLocation.top + (LONG)(rect.size[1] * ratio[1]);

			
			//doRender(text, textLocation, color, flags);
			rm.addText(boost::bind(&FontImpl::doRender, this, text, textLocation, color, flags));
		}		

		virtual void onLostDevice()
		{
			if (m_pFont != NULL)
			{
				m_pFont->OnLostDevice();
				destroy();
			}
		}		

		virtual void onResetDevice()
		{
			create();
			if (m_pFont != NULL) 
				m_pFont->OnResetDevice();
		}

		virtual ~FontImpl()
		{
			destroy();

			if (0 == fontsCreated)
			{
				//TheFontRenderManager::destroy();
			}
		}	

	private:		
		ID3DXFont	*m_pFont;
	};

	font_ptr base_font::create(int height, const std::wstring &name, FontWeight font_weigh)
	{
		try
		{
			return font_ptr(new FontImpl(height, name, font_weigh));
		}
		catch (...)
		{
			return font_ptr();
		}
	}
}