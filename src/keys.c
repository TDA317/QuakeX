/*
Copyright (C) 1996-1997 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
#include "quakedef.h"

/*

key up events are sent even if in console mode

*/


#define		MAXCMDLINE	256
char	key_lines[32][MAXCMDLINE];
int		key_linepos;
int		shift_down=false;
int		key_lastpress;

int		edit_line=0;
int		history_line=0;

keydest_t	key_dest;

int		key_count;			// incremented every key event

char	*keybindings[256];
qboolean	consolekeys[256];	// if true, can't be rebound while in console
qboolean	menubound[256];	// if true, can't be rebound while in menu
qboolean	demokeys[256];				// Manoel Kasimier
qboolean	key_menurepeats[256];			// Manoel Kasimier
int		keyboard_active;			// Manoel Kasimier
int		M_OnScreenKeyboard_Key (int key);	// Manoel Kasimier
int		keyshift[256];		// key to map to if shift held down in console
int		key_repeats[256];	// if > 1, it is autorepeating
qboolean	keydown[256];

typedef struct
{
	char	*name;
	int		keynum;
} keyname_t;

keyname_t keynames[] =
{
	{"TAB", K_TAB},
	{"ENTER", K_ENTER},
	{"ESCAPE", K_ESCAPE},
	{"SPACE", K_SPACE},
	{"BACKSPACE", K_BACKSPACE},
	{"UPARROW", K_UPARROW},
	{"DOWNARROW", K_DOWNARROW},
	{"LEFTARROW", K_LEFTARROW},
	{"RIGHTARROW", K_RIGHTARROW},

	{"ALT", K_ALT},
	{"CTRL", K_CTRL},
	{"SHIFT", K_SHIFT},
	
	{"F1", K_F1},
	{"F2", K_F2},
	{"F3", K_F3},
	{"F4", K_F4},
	{"F5", K_F5},
	{"F6", K_F6},
	{"F7", K_F7},
	{"F8", K_F8},
	{"F9", K_F9},
	{"F10", K_F10},
	{"F11", K_F11},
	{"F12", K_F12},

	{"INS", K_INS},
	{"DEL", K_DEL},
	{"PGDN", K_PGDN},
	{"PGUP", K_PGUP},
	{"HOME", K_HOME},
	{"END", K_END},

	{"MOUSE1", K_MOUSE1},
	{"MOUSE2", K_MOUSE2},
	{"MOUSE3", K_MOUSE3},

	{"JOY1", K_JOY1},
	{"JOY2", K_JOY2},
	{"JOY3", K_JOY3},
	{"JOY4", K_JOY4},

	{"XBOX_A", K_XBOX_A},
	{"XBOX_B", K_XBOX_B},
	{"XBOX_X", K_XBOX_X},
	{"XBOX_Y", K_XBOX_Y},
	{"XBOX_BACK", K_XBOX_BACK},
	{"XBOX_START", K_XBOX_START},
	{"XBOX_UP", K_DPAD_UP},
	{"XBOX_DOWN", K_DPAD_DOWN},
	{"XBOX_LEFT", K_DPAD_LEFT},
	{"XBOX_RIGHT", K_DPAD_RIGHT},
	{"XBOX_LTRIG", K_XBOX_LTRIG},
	{"XBOX_RTRIG", K_XBOX_RTRIG}, 
	// TDA Added Start 3/18/04
	// Seems Black and White arent supported
	// Lets try to add them anyway
	{"XBOX_BLACK", K_XBOX_BLACK},
	{"XBOX_WHITE", K_XBOX_WHITE},
	// TDA Added End 3/18/04

	{"AUX13", K_AUX13},
	{"AUX14", K_AUX14},
	{"AUX15", K_AUX15},
	{"AUX16", K_AUX16},
	{"AUX17", K_AUX17},
	{"AUX18", K_AUX18},
	{"AUX19", K_AUX19},
	{"AUX20", K_AUX20},
	{"AUX21", K_AUX21},
	{"AUX22", K_AUX22},
	{"AUX23", K_AUX23},
	{"AUX24", K_AUX24},
	{"AUX25", K_AUX25},
	{"AUX26", K_AUX26},
	{"AUX27", K_AUX27},
	{"AUX28", K_AUX28},
	{"AUX29", K_AUX29},
	{"AUX30", K_AUX30},
	{"AUX31", K_AUX31},
	{"AUX32", K_AUX32},

	{"PAUSE", K_PAUSE},

	{"MWHEELUP", K_MWHEELUP},
	{"MWHEELDOWN", K_MWHEELDOWN},

	{"SEMICOLON", ';'},	// because a raw semicolon seperates commands

	{NULL,0}
};

/*
==============================================================================

			LINE TYPING INTO THE CONSOLE

==============================================================================
*/


/*
====================
Key_Console

Interactive line editing and console scrollback
====================
*/
void Key_Console (int key)
{
	char	*cmd;

	key = M_OnScreenKeyboard_Key (key); // Manoel Kasimier

	
	if (key == K_ENTER || key == K_XBOX_START) // Manoel Kasimier - edited Re edited by TDA
//	if (key == K_ENTER)
	{
		Cbuf_AddText (key_lines[edit_line]+1);	// skip the >
		Cbuf_AddText ("\n");
		Con_Printf ("%s\n",key_lines[edit_line]);
		edit_line = (edit_line + 1) & 31;
		history_line = edit_line;
		key_lines[edit_line][0] = ']';
		key_linepos = 1;
		if (cls.state == ca_disconnected)
			SCR_UpdateScreen ();	// force an update, because the command
									// may take some time
		return;
	}

	// TDA Removed Start 3/18/04
	// Replaced this with the 'if' that follows
	// Better tab completion
	// http://www.quakesrc.org/?Page=tutorials&What=./tutorials/engine/console/console5.txt
	/*
	if (key == K_TAB)
	{	// command completion
		cmd = Cmd_CompleteCommand (key_lines[edit_line]+1);
		if (!cmd)
			cmd = Cvar_CompleteVariable (key_lines[edit_line]+1);
		if (cmd)
		{
			Q_strcpy (key_lines[edit_line]+1, cmd);
			key_linepos = Q_strlen(cmd)+1;
			key_lines[edit_line][key_linepos] = ' ';
			key_linepos++;
			key_lines[edit_line][key_linepos] = 0;
			return;
		}
	}
	*/
	// TDA Removed End 3/18/04
		
	// TDA Added Start 3/18/04
	// Better tab completion
	// http://www.quakesrc.org/?Page=tutorials&What=./tutorials/engine/console/console5.txt
	if (key == K_TAB)
	{
		// Enhanced console command completion [Fett] <start>

		/*IDCODE
		cmd = Cmd_CompleteCommand (key_lines[edit_line]+1);

		if (!cmd)
			cmd = Cvar_CompleteVariable (key_lines[edit_line]+1);
		*/

		int c; 
		int v;
		int a;

		// Count the number of possible matches

		c = Cmd_CompleteCountPossible (key_lines[edit_line]+1);
		v = Cvar_CompleteCountPossible (key_lines[edit_line]+1);
		a = Cmd_CompleteAliasCountPossible (key_lines[edit_line]+1);

		if (!(c + v + a))		// No possible matches, don't do anything
				return;

		if (c + v > 1)	// More than a single possible match
		{
			// the 'classic' Quakebar
			Con_Printf("\n\35\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\37\n");

			// Print possible commands

			if (c)
			{
				if (c==1)
					Con_Printf("1 possible command:\n");
				else
					Con_Printf("%i possible commands:\n", c);

				Cmd_CompletePrintPossible (key_lines[edit_line]+1);
			}

			// Print possible variables

			if (v)
			{
				if (v==1)
					Con_Printf("1 possible variable:\n");
				else
					Con_Printf("%i possible variables:\n", v);

				Cvar_CompletePrintPossible (key_lines[edit_line]+1);
			}

			// Print possible aliases

			if (a)
			{
				if (a==1)
					Con_Printf("1 possible alias:\n");
				else
					Con_Printf("%i possible aliases:\n", a);

				Cmd_CompleteAliasPrintPossible (key_lines[edit_line]+1);
			}
			return;
		}

		// We know there's only one match so use the original id functions
		// to complete the line.

		if (c)
			cmd = Cmd_CompleteCommand (key_lines[edit_line]+1);

		if (v)
			cmd = Cvar_CompleteVariable (key_lines[edit_line]+1);

		if (a)
			cmd = Cmd_CompleteAlias (key_lines[edit_line]+1);


		// Enhanced console command completion [Fett] <end>

		if(cmd)
		{
			Q_strcpy (key_lines[edit_line]+1, cmd);
			key_linepos = Q_strlen(cmd)+1;
			key_lines[edit_line][key_linepos] = ' ';
			key_linepos++;
			key_lines[edit_line][key_linepos] = 0;
			return;
		}
	}
	// TDA Added End 3/18/04
	
	if (key == K_BACKSPACE || key == K_LEFTARROW)
	{
		if (key_linepos > 1)
			key_linepos--;
		return;
	}

	if (key == K_UPARROW || key == K_DPAD_LEFT)  // Manoel Kasimier - edited Re edited by TDA
//	if (key == K_UPARROW)
	{
		do
		{
			history_line = (history_line - 1) & 31;
		} while (history_line != edit_line
				&& !key_lines[history_line][1]);
		if (history_line == edit_line)
			history_line = (edit_line+1)&31;
		Q_strcpy(key_lines[edit_line], key_lines[history_line]);
		key_linepos = Q_strlen(key_lines[edit_line]);
		return;
	}

	if (key == K_DOWNARROW || key == K_DPAD_RIGHT) // Manoel Kasimier - edited Re edited by TDA
//	if (key == K_DOWNARROW)
	{
		if (history_line == edit_line) return;
		do
		{
			history_line = (history_line + 1) & 31;
		}
		while (history_line != edit_line
			&& !key_lines[history_line][1]);
		if (history_line == edit_line)
		{
			key_lines[edit_line][0] = ']';
			key_linepos = 1;
		}
		else
		{
			Q_strcpy(key_lines[edit_line], key_lines[history_line]);
			key_linepos = Q_strlen(key_lines[edit_line]);
		}
		return;
	}

	if (key == K_PGUP || key==K_MWHEELUP || key == K_DPAD_UP) // Manoel Kasimier - edited Re edited by TDA
//	if (key == K_PGUP || key==K_MWHEELUP)
	{
		con_backscroll += 2;
		if (con_backscroll > con_totallines - (vid.height>>3) - 1)
			con_backscroll = con_totallines - (vid.height>>3) - 1;
		return;
	}

	if (key == K_PGDN || key==K_MWHEELDOWN || key == K_DPAD_DOWN) // Manoel Kasimier - edited Re edited by TDA
//	if (key == K_PGDN || key==K_MWHEELDOWN)
	{
		con_backscroll -= 2;
		if (con_backscroll < 0)
			con_backscroll = 0;
		return;
	}
	if (key == K_HOME || key == K_XBOX_RTRIG) // Manoel Kasimier - edited Re edited by TDA
//	if (key == K_HOME)
	{
		con_backscroll = con_totallines - (vid.height>>3) - 1;
		return;
	}

	if (key == K_END || key == K_XBOX_LTRIG) // Manoel Kasimier - edited Re edited by TDA
//	if (key == K_END)
	{
		con_backscroll = 0;
		return;
	}
	
	if (key < 32 || key > 127)
		return;	// non printable
		
	if (key_linepos < MAXCMDLINE-1)
	{
		key_lines[edit_line][key_linepos] = key;
		key_linepos++;
		key_lines[edit_line][key_linepos] = 0;
	}

}

//============================================================================

char chat_buffer[32];
qboolean team_message = false;

void Key_Message (int key)
{
	static int chat_bufferlen = 0;

	key = M_OnScreenKeyboard_Key (key); // Manoel Kasimier	

	if (key == K_ENTER)
	{
		if (team_message)
			Cbuf_AddText ("say_team \"");
		else
			Cbuf_AddText ("say \"");
		Cbuf_AddText(chat_buffer);
		Cbuf_AddText("\"\n");

		key_dest = key_game;
		chat_bufferlen = 0;
		chat_buffer[0] = 0;
		return;
	}

//	if (key == K_ESCAPE)
//	{
	if (key == K_ESCAPE || key == K_XBOX_B || key == K_XBOX_BACK) // Manoel Kasimier - edited Re edited by TDA
	{
		M_OnScreenKeyboard_Reset(); // Manoel Kasimier

		key_dest = key_game;
		chat_bufferlen = 0;
		chat_buffer[0] = 0;
		return;
	}

	if (key < 32 || key > 127)
		return;	// non printable

	if (key == K_BACKSPACE)
	{
		if (chat_bufferlen)
		{
			chat_bufferlen--;
			chat_buffer[chat_bufferlen] = 0;
		}
		return;
	}

	if (chat_bufferlen == 31)
		return; // all full

	chat_buffer[chat_bufferlen++] = key;
	chat_buffer[chat_bufferlen] = 0;
}

//============================================================================


/*
===================
Key_StringToKeynum

Returns a key number to be used to index keybindings[] by looking at
the given string.  Single ascii characters return themselves, while
the K_* names are matched up.
===================
*/
int Key_StringToKeynum (char *str)
{
	keyname_t	*kn;
	
	if (!str || !str[0])
		return -1;
	if (!str[1])
		return str[0];

	for (kn=keynames ; kn->name ; kn++)
	{
		if (!Q_strcasecmp(str,kn->name))
			return kn->keynum;
	}
	return -1;
}

/*
===================
Key_KeynumToString

Returns a string (either a single ascii char, or a K_* name) for the
given keynum.
FIXME: handle quote special (general escape sequence?)
===================
*/
char *Key_KeynumToString (int keynum)
{
	keyname_t	*kn;	
	static	char	tinystr[2];
	
	if (keynum == -1)
		return "<KEY NOT FOUND>";
	if (keynum > 32 && keynum < 127)
	{	// printable ascii
		tinystr[0] = keynum;
		tinystr[1] = 0;
		return tinystr;
	}
	
	for (kn=keynames ; kn->name ; kn++)
		if (keynum == kn->keynum)
			return kn->name;

	return "<UNKNOWN KEYNUM>";
}


/*
===================
Key_SetBinding
===================
*/
void Key_SetBinding (int keynum, char *binding)
{
	char	*new;
	int		l;
			
	if (keynum == -1)
		return;

// free old bindings
	if (keybindings[keynum])
	{
		Z_Free (keybindings[keynum]);
		keybindings[keynum] = NULL;
	}
			
// allocate memory for new binding
	l = Q_strlen (binding);	
	new = Z_Malloc (l+1);
	Q_strcpy (new, binding);
	new[l] = 0;
	keybindings[keynum] = new;	
}

/*
===================
Key_Unbind_f
===================
*/
void Key_Unbind_f (void)
{
	int		b;

	if (Cmd_Argc() != 2)
	{
		Con_Printf ("unbind <key> : remove commands from a key\n");
		return;
	}
	
	b = Key_StringToKeynum (Cmd_Argv(1));
	if (b==-1)
	{
		Con_Printf ("\"%s\" isn't a valid key\n", Cmd_Argv(1));
		return;
	}

	Key_SetBinding (b, "");
}

void Key_Unbindall_f (void)
{
	int		i;
	
	for (i=0 ; i<256 ; i++)
		if (keybindings[i])
			Key_SetBinding (i, "");
}


/*
===================
Key_Bind_f
===================
*/
void Key_Bind_f (void)
{
	int			i, c, b;
	char		cmd[1024];
	
	c = Cmd_Argc();

	if (c != 2 && c != 3)
	{
		Con_Printf ("bind <key> [command] : attach a command to a key\n");
		return;
	}
	b = Key_StringToKeynum (Cmd_Argv(1));
	if (b==-1)
	{
		Con_Printf ("\"%s\" isn't a valid key\n", Cmd_Argv(1));
		return;
	}

	if (c == 2)
	{
		if (keybindings[b])
			Con_Printf ("\"%s\" = \"%s\"\n", Cmd_Argv(1), keybindings[b] );
		else
			Con_Printf ("\"%s\" is not bound\n", Cmd_Argv(1) );
		return;
	}
	
// copy the rest of the command line
	cmd[0] = 0;		// start out with a null string
	for (i=2 ; i< c ; i++)
	{
		if (i > 2)
			strcat (cmd, " ");
		strcat (cmd, Cmd_Argv(i));
	}

	Key_SetBinding (b, cmd);
}

/*
============
Key_WriteBindings

Writes lines containing "bind key value"
============
*/
void Key_WriteBindings (FILE *f)
{
	int		i;

	for (i=0 ; i<256 ; i++)
		if (keybindings[i])
			if (*keybindings[i])
				fprintf (f, "bind \"%s\" \"%s\"\n", Key_KeynumToString(i), keybindings[i]);
}


/*
===================
Key_Init
===================
*/
void Key_Init (void)
{
	int		i;

	for (i=0 ; i<32 ; i++)
	{
		key_lines[i][0] = ']';
		key_lines[i][1] = 0;
	}
	key_linepos = 1;
	
//
// init ascii characters in console mode
//
	for (i=32 ; i<128 ; i++)
		consolekeys[i] = true;
	consolekeys[K_ENTER] = true;
	consolekeys[K_TAB] = true;
	consolekeys[K_LEFTARROW] = true;
	consolekeys[K_RIGHTARROW] = true;
	consolekeys[K_UPARROW] = true;
	consolekeys[K_DOWNARROW] = true;
	consolekeys[K_BACKSPACE] = true;
	consolekeys[K_HOME] = true; // Manoel Kasimier
	consolekeys[K_END] = true; // Manoel Kasimier
	consolekeys[K_PGUP] = true;
	consolekeys[K_PGDN] = true;
	consolekeys[K_SHIFT] = true;
	consolekeys[K_MWHEELUP] = true;
	consolekeys[K_MWHEELDOWN] = true;
	consolekeys['`'] = false;
	consolekeys['~'] = false;
	// Manoel Kasimier - begin

// TDA Changed from Fragger's source Start
// For some reason Fragger's way causes the dpad presses to go at mach speed in QuakeX
// Original is commented out below and the replacement is below that.
/*
	consolekeys[K_XBOX_A] = true;
	consolekeys[K_XBOX_B] = true;
	consolekeys[K_XBOX_X] = true;
	consolekeys[K_XBOX_Y] = true;
	consolekeys[K_XBOX_LTRIG] = true;
	consolekeys[K_XBOX_RTRIG] = true;
	consolekeys[K_XBOX_START] = true;
	consolekeys[K_XBOX_BACK] = true; // TDA added
	consolekeys[K_DPAD_LEFT] = true;
	consolekeys[K_DPAD_DOWN] = true;
	consolekeys[K_DPAD_RIGHT] = true;
	consolekeys[K_DPAD_UP] = true;
*/

// TDA Changed from Fragger's source
	consolekeys[K_XBOX_A] = true;
	consolekeys[K_XBOX_B] = false;
	consolekeys[K_XBOX_X] = false;
	consolekeys[K_XBOX_Y] = false;
	consolekeys[K_XBOX_LTRIG] = false;
	consolekeys[K_XBOX_RTRIG] = false;
	consolekeys[K_XBOX_START] = false;
	consolekeys[K_XBOX_BACK] = false; // TDA added
	consolekeys[K_DPAD_LEFT] = false;
	consolekeys[K_DPAD_DOWN] = false;
	consolekeys[K_DPAD_RIGHT] = false;
	consolekeys[K_DPAD_UP] = false;
// TDA Changed from Fragger's source END

	// Manoel Kasimier - end


	for (i=0 ; i<256 ; i++)
		keyshift[i] = i;
	for (i='a' ; i<='z' ; i++)
		keyshift[i] = i - 'a' + 'A';
	keyshift['1'] = '!';
	keyshift['2'] = '@';
	keyshift['3'] = '#';
	keyshift['4'] = '$';
	keyshift['5'] = '%';
	keyshift['6'] = '^';
	keyshift['7'] = '&';
	keyshift['8'] = '*';
	keyshift['9'] = '(';
	keyshift['0'] = ')';
	keyshift['-'] = '_';
	keyshift['='] = '+';
	keyshift[','] = '<';
	keyshift['.'] = '>';
	keyshift['/'] = '?';
	keyshift[';'] = ':';
	keyshift['\''] = '"';
	keyshift['['] = '{';
	keyshift[']'] = '}';
	keyshift['`'] = '~';
	keyshift['\\'] = '|';

	menubound[K_ESCAPE] = true;
	for (i=0 ; i<12 ; i++)
		menubound[K_F1+i] = true;


	// Manoel Kasimier - begin
	// TDA Edited to match XBOX
	for (i=0 ; i<200 ; i++)
		demokeys[i] = true;
	demokeys[K_PAUSE] = true;
	demokeys[K_XBOX_START] = true;
	for (i=0 ; i<12 ; i++)
		demokeys[K_F1+i] = false;
	demokeys['`'] = false;
	demokeys['~'] = false;

// TDA Changed from Fragger's source Start
// For some reason Fragger's way causes the dpad presses to go at mach speed in QuakeX
// Original is commented out below and the replacement is below that.
/*
	key_menurepeats[K_UPARROW] = true;
	key_menurepeats[K_DOWNARROW] = true;
	key_menurepeats[K_LEFTARROW] = true;
	key_menurepeats[K_RIGHTARROW] = true;
	key_menurepeats[K_DPAD_LEFT] = true;
	key_menurepeats[K_DPAD_DOWN] = true;
	key_menurepeats[K_DPAD_RIGHT] = true;
	key_menurepeats[K_DPAD_UP] = true;
*/

	key_menurepeats[K_UPARROW] = true;
	key_menurepeats[K_DOWNARROW] = true;
	key_menurepeats[K_LEFTARROW] = true;
	key_menurepeats[K_RIGHTARROW] = true;
	key_menurepeats[K_DPAD_LEFT] = false;
	key_menurepeats[K_DPAD_DOWN] = false;
	key_menurepeats[K_DPAD_RIGHT] = false;
	key_menurepeats[K_DPAD_UP] = false; 
// TDA Changed from Fragger's source END
	// Manoel Kasimier - end


//
// register our functions
//
	Cmd_AddCommand ("bind",Key_Bind_f);
	Cmd_AddCommand ("unbind",Key_Unbind_f);
	Cmd_AddCommand ("unbindall",Key_Unbindall_f);


}

/*
===================
Key_Event

Called by the system between frames for both key up and key down events
Should NOT be called during an interrupt!
===================
*/
void Key_Event (int key, qboolean down)
{
	char	*kb;
	char	cmd[1024];

	keydown[key] = down;

	if (!down)
		key_repeats[key] = 0;

	key_lastpress = key;
	key_count++;
	if (key_count <= 0)
	{
		return;		// just catching keys for Con_NotifyBox
	}

// update auto-repeat status
	if (down)
	{
		key_repeats[key]++;
		if (key_repeats[key] > 1) // Manoel Kasimier - edited
		if (!(key_dest == key_menu && key_menurepeats[key])) // Manoel Kasimier
		if (!(key_dest == key_console && consolekeys[key])) // Manoel Kasimier
//		if (key != K_BACKSPACE && key != K_PAUSE && key_repeats[key] > 1)  // Monoel Kasimier Removed
		{
			return;	// ignore most autorepeats
		}
			
//		if (key >= 200 && !keybindings[key]) // Manoel Kasimier - removed
//			Con_Printf ("%s is unbound, hit F4 to set.\n", Key_KeynumToString (key) ); // Manoel Kasimier - removed
	}

	if (key == K_SHIFT)
		shift_down = down;

//
// handle escape specialy, so the user can never unbind it
//
	if (key == K_ESCAPE || key == K_XBOX_BACK)
	{
		if (!down)
			return;
		switch (key_dest)
		{
		case key_message:
			Key_Message (key);
			break;
		case key_menu:
			M_Keydown (key);
			break;
		case key_game:
		case key_console:
			M_ToggleMenu_f ();
			break;
		default:
			Sys_Error ("Bad key_dest");
		}
		return;
	}

	// Manoel Kasimier - begin
	if (key_dest == key_console)
		if (((key == K_XBOX_B && !keyboard_active) || key == K_XBOX_START) && down)
		{
			keyboard_active	= 0;
			M_ToggleMenu_f ();
			return;
		}
	// Manoel Kasimier - end	


//
// key up events only generate commands if the game key binding is
// a button command (leading + sign).  These will occur even in console mode,
// to keep the character from continuing an action started before a console
// switch.  Button commands include the kenum as a parameter, so multiple
// downs can be matched with ups
//
	if (!down)
	{
		kb = keybindings[key];
		if (kb && kb[0] == '+')
		{
			sprintf (cmd, "-%s %i\n", kb+1, key);
			Cbuf_AddText (cmd);
		}
		if (keyshift[key] != key)
		{
			kb = keybindings[keyshift[key]];
			if (kb && kb[0] == '+')
			{
				sprintf (cmd, "-%s %i\n", kb+1, key);
				Cbuf_AddText (cmd);
			}
		}
		return;
	}

//
// during demo playback, most keys bring up the main menu
//
//	if (cls.demoplayback && down && consolekeys[key] && key_dest == key_game) // Manoel Kasimier - edited
	if (cls.demoplayback && down && demokeys[key] && key_dest == key_game) // Manoel Kasimier - edited
	{
		M_ToggleMenu_f ();
		return;
	}

//
// if not a consolekey, send to the interpreter no matter what mode is
//
	if ( (key_dest == key_menu && menubound[key])
	|| (key_dest == key_console && !consolekeys[key])
	|| (key_dest == key_game && ( !con_forcedup || !consolekeys[key] ) ) )
	{
		kb = keybindings[key];
		if (kb)
		{
			if (kb[0] == '+')
			{	// button commands add keynum as a parm
				sprintf (cmd, "%s %i\n", kb, key);
				Cbuf_AddText (cmd);
			}
			else
			{
				Cbuf_AddText (kb);
				Cbuf_AddText ("\n");
			}
		}
		return;
	}

	if (!down)
		return;		// other systems only care about key down events

	if (shift_down)
	{
		key = keyshift[key];
	}

	switch (key_dest)
	{
	case key_message:
		Key_Message (key);
		break;
	case key_menu:
		M_Keydown (key);
		break;

	case key_game:
	case key_console:
		Key_Console (key);
		break;
	default:
		Sys_Error ("Bad key_dest");
	}
}


/*
===================
Key_ClearStates
===================
*/
void Key_ClearStates (void)
{
	int		i;

	for (i=0 ; i<256 ; i++)
	{
		keydown[i] = false;
		key_repeats[i] = 0;
	}
}

