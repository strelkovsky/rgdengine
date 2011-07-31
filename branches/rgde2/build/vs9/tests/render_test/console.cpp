#include "stdafx.h"
#include "console.h"

#include <boost/lexical_cast.hpp>

#include <rgde/render/device.h>

#include <stdarg.h>

//TODO!
#include <windows.h>

float GDeltaTime = 0.01f;

// Const declarations 
namespace 
{
	float LINE_TIME = 6; // Each line may remain for LINE_TIME seconds
	int MAX_LINES = 5;
	float lastLineTime = LINE_TIME; // How much time left for the oldest line

	const int g_YStart = 780;			// Chat y offset
	const int g_ConLineNum = 12;			// Half number chat lines
	const int g_Xoff = 10;				// Chat x offset

	const int g_ChatStringLength = 450; // Chat x stretch

	void trimLeft(std::string &value)
	{
		std::string::size_type where = value.find_first_not_of(' ');
		if (where == std::string::npos)
			/// string has nothing but space
			value = ("");
		else if (where != 0)
			value = value.substr(where);


		where = value.find_first_not_of('\t');
		if (where == std::string::npos)
			/// string has nothing but space
			value = ("");
		else if (where != 0)
			value = value.substr(where);
	}

	//-----------------------------------------------------------------------------
	// Trims space from right
	//-----------------------------------------------------------------------------
	void trimRight(std::string &value)
	{ 
		std::string::size_type where = value.find_last_not_of(' ');
		if (where == std::string::npos)
			/// string has nothing but space
			value = ("");
		else if (where != (value.length() - 1))
			value = value.substr(0, where + 1);

		where = value.find_last_not_of('\t');
		if (where == std::string::npos)
			/// string has nothing but space
			value = ("");
		else if (where != (value.length() - 1))
			value = value.substr(0, where + 1);
	}

}


namespace rgde
{
	namespace game
	{

		void console::show(bool ChatLineOnly)
		{
			is_console_on = true;
			m_is_chat_line_on = ChatLineOnly;
		}

		void console::hide()
		{
			is_console_on = false;
			m_is_chat_line_on = false;
		}

		//--------------------------------------------------------------------------------------
		//  prints a line of text to the console
		//--------------------------------------------------------------------------------------
		void console::printf(const char *fmt, ...)
		{
			va_list		argptr;
			char		msg[1024];
			va_start (argptr,fmt);
			vsprintf_s(msg,fmt,argptr);
			va_end (argptr);
			add_history_line(std::string(msg));
		}

		//--------------------------------------------------------------------------------------
		// prints a colored line of text to the console 
		//--------------------------------------------------------------------------------------
		void console::printf(math::color c, const char *fmt, ...)
		{
			va_list		argptr;
			char		msg[1024];
			va_start (argptr,fmt);
			vsprintf_s(msg,fmt,argptr);
			va_end (argptr);
			add_history_line(std::string(msg),c);
		}

		void console::printf(message_type type,int team,math::color color,const char *fmt, ...)
		{
			va_list		argptr;
			char		msg[1024];
			va_start (argptr,fmt);
			vsprintf_s(msg,fmt,argptr);
			va_end (argptr);
			add_history_line(type,team,std::string(msg),color);
		}

		console::console(render::device& dev) 
			: is_was_toggled(false)
			, m_device(dev)
			, m_canvas(dev)
		{
			m_draw_chat = true;
			m_is_allow_toggling = true;

			init("", "");

			m_small_font = render::font::create(dev, 9, L"Arial");
			m_medium_font = render::font::create(dev, 12, L"Arial");
		}

		console::~console()
		{
			is_console_on = false;
		}

		//--------------------------------------------------------------------------------------
		// adds entry to the console's history list of previously displayed lines
		//--------------------------------------------------------------------------------------
		void console::add_history_line(std::string& line,math::color color)
		{
			add_history_line(mt_console,-1,line,color);
		}

		void console::add_history_line(message_type type,int team,std::string& line, math::color color)
		{
			//LogPrintf(("Con: "+line).c_str());
			history_line histLine;
			//histLine.text = FromUTF8(line);
			histLine.text = xml::as_utf16(line.c_str());
			histLine.color = color;
			histLine.team = team;
			histLine.type = type;
			m_history_lines.push_back(histLine);
			m_recent_lines++;
		}


		void console::clear()
		{
			m_recent_lines = 0;
			m_history_lines.resize(0);
		}

		//--------------------------------------------------------------------------------------
		// Draws the console system onto the canvas
		//--------------------------------------------------------------------------------------
		void console::render()
		{
			//// Get the height of the font set using a dummy call
			int font_height = m_medium_font->measure_text(L"T").h;

			render::view_port vp = m_device.viewport();

			int screenWidth = vp.width;
			int screenHeight = vp.height;

			// dont need for old styled chat render
			if (m_is_chat_line_on)
			{
			//	render only history us chat echo

				int startY = g_YStart;
				int yOff = startY+font_height*g_ConLineNum;

				int xOff = g_Xoff;
				int yUp = yOff - 2*font_height;

				int YExt = startY-font_height*g_ConLineNum;

				for(int i=0;i<m_history_lines.size();i++)
				{
					yUp -= (font_height*(i+2));
					// We've gone too high up, draw ... and stop
					if(yUp <= YExt)
					{
						m_medium_font->render(L"...", math::rect(xOff,yUp, 100, 100), 0xFF00FA00, false);

						yUp -= font_height;
						break;
					}

					if (m_history_lines[m_history_lines.size()-(i+1)].text.empty())
						continue;

					math::color DrawColor = m_history_lines[m_history_lines.size()-(i+1)].color;

					std::list<std::wstring> DrawMessages;

					std::wstring DrawMessage;
					int j = 0;
					math::rect FontSize;

					while (m_history_lines[m_history_lines.size()-(i+1)].text.size() > j)
					{
						do
						{
							DrawMessage.push_back(*(m_history_lines[m_history_lines.size()-(i+1)].text.c_str() + j));
							FontSize  = m_medium_font->measure_text(DrawMessage);
							++j;
						} while ((g_ChatStringLength > FontSize.w) &&
							(m_history_lines[m_history_lines.size()-(i+1)].text.size() > j));

						if (!DrawMessage.empty())
						{
							DrawMessages.push_front(DrawMessage);
							DrawMessage.clear();
						}
					}

					for (std::list<std::wstring>::iterator It = DrawMessages.begin();
						It != DrawMessages.end();
						++It)
					{
						m_small_font->render(*It, math::rect(xOff,yUp, 600, 100), DrawColor, false);
						yUp -= font_height;
					}

					DrawMessages.clear();
				}

				//return;
			}
			else
			{
				// Console up, check/draw any recent lines to viewport
				if(!is_console_on/* || m_is_chat_line_on*/)
				{
					if(!m_draw_chat)
						return;

					int startY = 40;

					if(m_is_chat_line_on)
					{
						int yOff = startY+font_height*6;
						int xOff = 10;
						std::string displayString = std::string("> ") + editing_line;
						m_medium_font->render(displayString, math::rect(xOff,yOff, 1000, 1000), math::color(255,255,50,200), false);
						int x_pos = m_medium_font->measure_text(displayString.substr(0,cursor_pos+2)).w;
						m_medium_font->render(L"_", math::rect(xOff+x_pos,yOff, 1000, 1000), math::color(255,255,50,200), false);
					}

					int fadeLineNum = m_history_lines.size() - m_recent_lines - 1;
					if(fadeLineNum > -1 && fadeLineNum < m_history_lines.size() && m_fade_line_opacity_factor > 0)
					{
						math::color textColor = m_history_lines[fadeLineNum].color;
						textColor.a *= m_fade_line_opacity_factor;
						m_medium_font->render(m_history_lines[fadeLineNum].text, math::rect(10,startY, 1000, 1000), textColor, false);
						m_fade_line_opacity_factor -= GDeltaTime*1.2;
					}

					// Check for recent lines to draw directly to view
					if(m_recent_lines<=0)
						return;

					if(m_recent_lines>MAX_LINES)
					{
						m_fade_line_opacity_factor = 1;
						m_recent_lines = MAX_LINES;
					}

					// Draw remaining lines
					for(int i=0;i<m_recent_lines;i++)
					{
						int yUp = startY+font_height+(font_height*i);
						const history_line& _line  = m_history_lines[(m_history_lines.size()-m_recent_lines)+i];
						m_medium_font->render(_line.text, 10,yUp, _line.color);
					}

					// Remove oldest line when expired
					// Update timer
					lastLineTime -= GDeltaTime;
					if(lastLineTime <= 0)
					{

						m_recent_lines--;
						m_fade_line_opacity_factor = 1;
						if(m_recent_lines>=0)
							lastLineTime = LINE_TIME;
					}
					return;
				}
			}

			if( is_console_on && !m_is_chat_line_on )
			{
				/* The console is on, so draw all of its text and graphics */

				//set the viewport to front of Z buffer for canvas draws
				//int minZ = RenderDevice::Instance()->MinViewportZ;
				//int maxZ = RenderDevice::Instance()->MaxViewportZ;
				//D3DVIEWPORT9 viewport;
				//viewport.X = 0;
				//viewport.Y = 0;
				//viewport.MinZ = 0;
				//viewport.MaxZ = .05;
				//viewport.Height = Canvas::Instance()->Height;
				//viewport.Width = Canvas::Instance()->Width;
				//RenderDevice::Instance()->dev->SetViewport(&viewport);

				int xOff = 15;
				int yOff = 1200/3 - font_height-4;

				//// Draw the console Quads
				//{
				//	float conWidth =  1600;
				//	float conHeight = 1200/3;
				//	float x = -1, y = -1; 

				//	// Crude texture animation
				//	static float t = 0;
				//	//textures[0]->setUOffset(textures[0]->getUOffset() + GDeltaTime * 0.1f);
				//	//textures[0]->setVOffset(textures[0]->getVOffset() + GDeltaTime * 0.1f);

				//	// Alpha from diffuse
				//	RenderDevice::Instance()->SetTSS(0,D3DTSS_ALPHAOP , D3DTOP_BLENDDIFFUSEALPHA);
				//	RenderDevice::Instance()->SetTSS(0,D3DTSS_ALPHAARG1,D3DTA_DIFFUSE );

				//	canvas->Box(COLOR_ARGB(200,255, 255, 255),x,y,conWidth,conHeight,textures[0],BLEND_SRCALPHA,BLEND_INVSRCALPHA);

				//	// Normal texture coordinates - no tiling
				//	//textures[1]->setUTile(1);

				//	// Alpha from texture
				//	RenderDevice::Instance()->SetTSS(0,D3DTSS_ALPHAOP ,D3DTOP_SELECTARG1);
				//	RenderDevice::Instance()->SetTSS(0,D3DTSS_ALPHAARG1 ,D3DTA_TEXTURE   );

				//	canvas->Box(COLOR_ARGB(200,255, 255, 255),x,y,conWidth,conHeight,textures[1],BLEND_INVDESTCOLOR,BLEND_DESTCOLOR);

				//	RenderDevice::Instance()->SetTSS(0,D3DTSS_ALPHAOP, D3DTOP_MODULATE);
				//	RenderDevice::Instance()->SetTSS(0,D3DTSS_ALPHAARG1,D3DTA_TEXTURE );
				//	RenderDevice::Instance()->SetTSS(0,D3DTSS_ALPHAARG2,D3DTA_DIFFUSE );
				//}

				// Insertion point
				//m_small_font->render(">",xOff-13,yOff, math::color(255,0,220,0));
				m_small_font->render(">",math::rect(xOff-13,yOff, 1000, 1000), math::color(255,0,220,0));
				
				// Current Line
				//m_medium_font->render(editing_line,xOff,yOff, math::color(255,200,255,200));
				m_medium_font->render(editing_line,math::rect(xOff,yOff, 1000, 1000), math::color(255,200,255,200));
				

				// History Lines
				for(int i=0;i<m_history_lines.size();i++){
					int yUp = yOff-(font_height*(i+2));
					// We've gone too high up, draw ... and stop
					if(yUp <= 20){
						//canvas->Textf(MediumFont,COLOR_ARGB(255,0,220,0),xOff,yUp,"...");
						m_medium_font->render("...",xOff,yUp, math::color(255,0,220,0));
						break;
					}
					const history_line& _line = m_history_lines[m_history_lines.size()-(i+1)];
					m_medium_font->render(_line.text,xOff,yUp, _line.color);
				}

				static int count = 0;
				count++;

				// Blinking cursor
				if(count>50){
					//int x_pos = canvas->GetTextSize(MediumFont,editing_line.substr(0,cursor_pos).c_str()).cx;
					int x_pos = m_medium_font->measure_text(editing_line.substr(0,cursor_pos)).w;
					m_medium_font->render("_", xOff+x_pos,yOff, math::color(255,0,255,0));
				}
				if(count>100)
					count=0;

				//viewport.MinZ = minZ;
				//viewport.MaxZ = maxZ;
				//RenderDevice::Instance()->dev->SetViewport(&viewport);
			}
		}

		//--------------------------------------------------------------------------------------
		// Delete key; deletes character at cursor pos 
		//--------------------------------------------------------------------------------------
		void console::key_backspace(){
			// Return if there's nothing to delete
			if(editing_line == "" || cursor_pos == 0)
				return;

			// Erase one character
			editing_line.erase(cursor_pos-1,1);
			cursor_pos--;
		}

		//--------------------------------------------------------------------------------------
		// Down key; scrolls to most recent line
		//--------------------------------------------------------------------------------------
		void console::key_down(){

			// If this line is 1 beyond the end of history, it's the current line
			if(cur_history_line == m_typed_history.size()-1)
			{
				editing_line = old_edit_line;
				cursor_pos = editing_line.length();
				return;
			}

			// Can't go past most recent line (failsafe)
			if (cur_history_line < 0 || m_typed_history.size() < cur_history_line + 2)
				return;

			cur_history_line++;

			editing_line = m_typed_history[cur_history_line];
			cursor_pos = editing_line.length();
		}

		//--------------------------------------------------------------------------------------
		// Up key; previous line is shown
		//--------------------------------------------------------------------------------------
		void console::key_up(){
			// We can't go further back
			if(cur_history_line == 0)
				return;

			cur_history_line--;

			editing_line = m_typed_history[cur_history_line];
			cursor_pos = editing_line.length();
		}

		//--------------------------------------------------------------------------------------
		// initialized during game init process
		//--------------------------------------------------------------------------------------
		void console::init(const std::string& frontTex, const std::string& backTex)
		{
			cursor_pos = 0;
			m_fade_line_opacity_factor = 0;
			is_console_on = false;
			m_recent_lines = 0;
			cur_history_line = 0;
			//textures[0] = RenderDevice::Instance()->GetTextureManager().Get(backTex);
			//textures[1] = RenderDevice::Instance()->GetTextureManager().Get(frontTex);
		}

		void console::toggle()
		{
			is_was_toggled = true;
			is_console_on = !is_console_on;
			m_is_chat_line_on = false;
			//Game::Instance()->SetCursorVisible(true);
		}

		//--------------------------------------------------------------------------------------
		// /* Windows Messages */
		//--------------------------------------------------------------------------------------
		void console::msg_char(unsigned int wParam){
			char chrKey = wParam;

			if(is_was_toggled)
			{		
				is_was_toggled = false;		
				return;
			}
			// Do nothing if the console is not on
			if(!is_console_on)
				return;

			// If key with letter pressed
			if( chrKey > 0 && isprint(chrKey)) 
			{
				// insert it to editing line
				std::string strChr = "";strChr+=chrKey;
				editing_line.insert( cursor_pos, strChr);
				old_edit_line = editing_line;
				cursor_pos++;
			}
		}

		//--------------------------------------------------------------------------------------
		// /* Windows Messages */
		//--------------------------------------------------------------------------------------
		void console::msg_keydown(unsigned int wParam){
			// Do nothing if the console is not on
			if(!is_console_on)
				return;

			bool bShift = GetKeyState(VK_SHIFT) & 0x8000;
			switch( wParam) {
				case VK_RETURN:  
					{
						if(is_console_on)key_return();
						break;
					}
				case VK_UP:
					if(is_console_on)
						key_up();
					break;
				case VK_DOWN:
					if(is_console_on)
						key_down();
					break;
				case VK_TAB:
					if(is_console_on)
						key_tab();
					break;
				case VK_BACK:
					key_backspace();
					break;
				case VK_DELETE:
					key_backspace();
					break;
				case VK_LEFT:
					if(is_console_on && cursor_pos > 0)
						cursor_pos--;  
					break;
				case VK_RIGHT:
					if(is_console_on && cursor_pos < editing_line.length())
						cursor_pos++;
					break;
			}
		}

		//--------------------------------------------------------------------------------------
		// Return key is pressed, line is processed
		//--------------------------------------------------------------------------------------
		void console::key_return()
		{
			// Remove any excess white space
			trimLeft(editing_line);
			trimRight(editing_line);

			// Ignore blank lines
			if(editing_line == ""){
				cursor_pos = 0;
				return;
			}

			// Add to typed history
			m_typed_history.push_back(editing_line);

			if(!m_is_chat_line_on)
			{
				// Add line to console history
				add_history_line(editing_line);
				// Handle line
				// If / send command, respond if bad command
				// else if Networked send chat
				//if(editing_line.find("/")==0)editing_line = editing_line.substr(1,editing_line.length());
				std::string rValue = m_shell.exec_symbol(editing_line);

				if(rValue.size() > 0)
					add_history_line(rValue);
			}

			// Reset line and cursor pos and history line (for up/down searching)
			editing_line = "";
			old_edit_line = editing_line;
			cursor_pos = 0;
			cur_history_line = m_typed_history.size();
		}

		//--------------------------------------------------------------------------------------
		// Tab key; shows commands
		//--------------------------------------------------------------------------------------
		void console::key_tab(){

			// Max 60 chrs per line
			std::string details,commandstr;
			std::string foundstr;
			int nNumOfFoundCommands=0;
			std::vector<std::string> AllCommands;

			for(size_t i=0;i<m_shell.get_num_symbols();i++)
			{
				commandstr=m_shell.get_symbol_name(i);
				if (commandstr.find(editing_line)==0)
				{
					nNumOfFoundCommands++;
					AllCommands.push_back(commandstr);
					foundstr=commandstr;
				}
			}
			if (nNumOfFoundCommands>0)
			{
				if (nNumOfFoundCommands==1)
				{
					editing_line=foundstr;
					cursor_pos=editing_line.length();
				}
				else
				{
					add_history_line(std::string("---------- Console Commands and Variables: ----------"));

					int nMinCommandsize=99999;
					for(int i=0;i<AllCommands.size()-1;i++)
					{
						int nLength;
						int nNumCompare=0;	
						nLength=AllCommands[i].length();
						if (AllCommands[i+1].length()<nLength)
						{
							nLength=AllCommands[i+1].length();
						}
						for (int j = 0; j < nLength ; j++)
						{
							if (AllCommands[i][j]==AllCommands[i+1][j])
							{
								nNumCompare++;
							}
						}
						if (nNumCompare<nMinCommandsize)
						{
							nMinCommandsize=nNumCompare;
							if (nNumCompare==0)
								break;
						}
					}

					if (editing_line.length()<nMinCommandsize)
					{
						editing_line=AllCommands[0].substr(0,nMinCommandsize);
						cursor_pos=editing_line.length();
					}

					for(int i=0;i<AllCommands.size();i++)
					{
						details += AllCommands[i] + "      ";
						if(details.length()>60)
						{
							add_history_line(details);
							details = "";
						}
					}
					if(details!="")
						add_history_line(details);
				}
			}
		}


		//--------------------------------------------------------------------------------------
		// registerd a var or function for use with the console exceution system
		//--------------------------------------------------------------------------------------
		void console::shell::decl_symbol(shell_type type, const std::string& name, void* pSymbol){
			// Build new symbol
			shell_symbol newSymbol;
			newSymbol.type = type;
			newSymbol.name = name;
			newSymbol.value = pSymbol;

			typedef void FUNC(const char*);
			if (type == st_function)
				newSymbol.function = boost::bind((FUNC*)pSymbol, _1);
			else
				newSymbol.function = 0;

			// Add to list
			symbols.push_back(newSymbol);
		}

		void console::shell::undecl(const std::string& name)
		{
			std::vector<shell_symbol>::iterator iter = symbols.begin();
			for (; iter != symbols.end(); ++iter)
			{
				shell_symbol& shellSymbol = *iter;
				if (shellSymbol.name == name)
				{
					symbols.erase(iter);
					return;
				}
			}
		}

		void console::shell::decl_func(const std::string& name, const boost::function<void(const char*)>& function)
		{
			shell_symbol newSymbol;
			newSymbol.type = st_function;
			newSymbol.value = 0;
			newSymbol.name = name;
			newSymbol.function = function;

			// Add to list
			symbols.push_back(newSymbol);
		}

		//--------------------------------------------------------------------------------------
		// Finds a specified command symbol within the execution shell, returns the index on the shell_symbol list
		//--------------------------------------------------------------------------------------
		int console::shell::find_cmd_name(std::string s)
		{
			//TODO:
			//ToLowerCase(s);
			boost::algorithm::to_lower(s);

			std::vector<shell_symbol>::iterator ppEachItem;
			int n = 0;
			for ( ppEachItem = symbols.begin();
				ppEachItem != symbols.end(); ppEachItem++ )
			{
				// Compare the name, ignoring any parenthesis
				std::string cmp = (*ppEachItem).name;
				if(cmp.find("(")!=-1)
					cmp = cmp.substr(0,cmp.find("("));
				//TODO:
				//ToLowerCase(cmp);
				if(cmp == s)
					return n;
				n++;
			}

			return -1;
		}

		//--------------------------------------------------------------------------------------
		// Returns symbol value if a var, converted to string
		//--------------------------------------------------------------------------------------
		std::string console::shell::get_symbol_value(shell_symbol symbol){
			char val[128];
			// Figure out the data value
			switch(symbol.type){
			case st_int:
				sprintf_s(val,"%d",*(int*)symbol.value);
				break;
			case st_stdstring:
				sprintf_s(val,"%s",((std::string*)symbol.value)->c_str());
				break;
			case st_float:
				sprintf_s(val,"%f",*(float*)symbol.value);
				break;
			case st_bool:
				sprintf_s(val,"%s",((*(bool*)symbol.value)? "true":"false"));
				break;
			case st_vector:
				{
					rgde::math::vec3f& v = *(rgde::math::vec3f*)symbol.value;
					sprintf_s(val,"%f %f %f",v[0], v[1], v[2]);
				}
				break;
			}
			return val;
		}

		//--------------------------------------------------------------------------------------
		// Parses & executes command, whether a function with params or setting of a var
		//--------------------------------------------------------------------------------------
		std::string console::shell::exec_symbol(std::string cmd)
		{
			try
			{
				std::string rvalue;
				std::string name;

				if(cmd.find("set ") == 0)
					cmd = cmd.substr(4);
				else
				{
					std::string::size_type i = cmd.find(" set ");
					if (i != std::string::npos)
						cmd = cmd.substr(i+5);
				}

				// Get the pure name
				if(cmd.find("(")!=-1)
					name = cmd.substr(0,cmd.find("("));
				else
					name = cmd.substr(0,cmd.find(" "));

				// Search for the pure name
				int index = find_cmd_name(name);
				if(index == -1)return "";

				shell_symbol symbol = symbols[index];

				// If it's a function execute it
				if(symbol.type == st_function)
				{
					if(cmd.find("=")!=-1)
						return "That's a command, you can't give it a value, silly!";
					// Extract the params, either in parenthesis or not, what do I care?
					std::string param;
					if(cmd.find("(")!=-1)
						param = cmd.substr(cmd.find("(")+1,cmd.find_last_of(")")-(cmd.find("(")+1));
					else
						param = cmd.substr(cmd.find(" ")+1,cmd.length());

					// No params
					if(param == "")
					{
						symbol.function("");
					}
					// params
					else{
						// TIM: Removed - so functions get numbers as strings
						// See if it's a number
						//char* s;
						//errno = 0;
						//int number = strtol(param.c_str(),&s,10);
						//if(!isdigit((int)param[0])) // It's not a number
						symbol.function(param.c_str());
						//else			// It's a number
						//	( (void (*)(int))symbol.value)(number);
					}
					return "Command Executed";
				}

				// If it's a var without assignment display value
				if(cmd.find(" ")==-1){
					return name + " = " + get_symbol_value(symbol);
				}
				// This is an assignment
				// Filter out the value part
				std::string value;
				if(cmd.find(" ")!=-1){
					value = cmd.substr(cmd.find(" ")+1,cmd.length());
				}
				else{
					value = cmd.substr(cmd.find("=")+1,cmd.length());
					//TODO:
					//trimLeft(value);
				}
				// Figure out the data type and then assign it
				switch(symbol.type){
				case st_int:
					*(int*)symbol.value = atoi(value.c_str());
					break;
				case st_stdstring:
					// This is SOOO EVIL
					*(std::string*)symbol.value = value;
					break;
				case st_float:
					*(float*)symbol.value = atof(value.c_str());
					break;
				case st_vector:
					{
						std::string sx = value.substr(0,value.find(" "));
						value = value.substr(value.find(" ")+1);
						std::string sy = value.substr(0,value.find(" "));
						value = value.substr(value.find(" ")+1);
						std::string sz = value.substr(0,value.find(" "));

						float x = atof(sx.c_str());
						float y = atof(sy.c_str());
						float z = atof(sz.c_str());

						(*(rgde::math::vec3f*)symbol.value) = rgde::math::vec3f(x, y, z);

						break;
					}

				case st_bool:{
					if(value.find("true")!=-1||value.find("1")!=-1)
						*(bool*)symbol.value = true;
					else if(value.find("false")!=-1||value.find("0")!=-1)
						*(bool*)symbol.value = false;
					else 
						return "Invalid value for bool. Use true/false or 1/0";
							 }
							 break;
				}
				return name +" set to: "+get_symbol_value(symbol);
			}
			catch (boost::bad_lexical_cast)
			{
				return "boost::bad_lexical_cast exception";
			}
		}

		bool	TestComment(const char* str)
		{
			if (*str == '#')
				return	true;
			if (*str == ';')
				return	true;
			if (*str++ == '/' && *str == '/')
				return	true;
			return	false;
		}

		void	console::exec_file(const char* filename)
		{
			return;
			//std::wstring	fullname; 
			//fullname = GetDefaultOutputDirectoryW() + L'\\' + FromUTF8(filename);

			//std::ifstream	file;
			//if (!FileExists(fullname)) 
			//{	
			//	std::string	fullname = filename;
			//	if (!FindMedia(fullname, ""))
			//	{
			//		Printf("file not found %s", filename);
			//		return;
			//	}else
			//	{
			//		file.open(fullname.c_str());
			//	}
			//}
			//else
			//{
			//	file.open(fullname.c_str());
			//}

			//std::string		line;

			//if (!file.is_open())
			//{
			//	Printf("failed to open file %s", filename);
			//	return;
			//}

			//while(!file.eof() && !file.fail())
			//{
			//	getline(file, line);
			//	size_t lineSize = line.size(); 
			//	for (size_t n = 0; n != lineSize; ++n)
			//	{
			//		if (TestComment(line.c_str() + n))
			//		{
			//			line.erase(n);
			//			break;
			//		}
			//	}
			//	if (line.empty())
			//		continue;
			//	exec_command(line);
			//}
		}

	}
}