# NAT2MID
Natsume (GB/GBC) to MIDI converter

This tool converts music from Game Boy and Game Boy Color games using Natsume's sound engine (probably programmed by Iku Mizutani) to MIDI format. There are two major editions of the driver, with the second containing major differences and possibly converted from MIDI.

It works with ROM images. To use it, you must specify the name of the ROM followed by the number of the bank containing the sound data (in hex). However, unlike most other converters, it is also required to specify the pointer to the song table, since I was unable to find a consistent method to look for it. You also should follow this by a third parameter denoting the "format" the game uses (1, 3, and 4 are almost identical (1 is for early games while 3 is for games from 1996 and later), while 2 has major differences and 4 only seems to be used for Ballistic).
For games that contain multiple banks of music, you must run the program multiple times specifying where each different bank is located. However, in order to prevent files from being overwritten, the MIDI files from the previous bank must either be moved to a separate folder or renamed. For very late games released in 2001 (format 2), the later version of the driver handles multiple banks with a single table, so this is not necessary for those games.

Note that for many games, there are "empty" tracks (usually the first or last track). This is normal.

Examples:
* NAT2MID "Spanky's Quest (E).gb" 2 4A5B 1
* NAT2MID "Tail Gator (U) [!].gb" 2 5F76 1
* NAT2MID "Medarot 2 - Kabuto Version (J) [C][!].gbc" 2 5C9A 3
* NAT2MID "Medarot 2 - Kabuto Version (J) [C][!].gbc" 3 5C9A 3
* NAT2MID "Action Man - Search for Base X (U) [C][!].gbc" 2 4D72 2

This tool was based on my own reverse-engineering of the sound engine, using Spanky's Quest and Action Man for reference. As usual, a "prototype" program, NAT2TXT, is also included.

Supported games:
  * Action Man: Search for Base X
  * Amazing Penguin
  * Ballistic
  * Bass Masters Classic
  * Croc 2
  * Dragon Dance
  * Hole in One Golf
  * Keitai Denjuu Telefang: Power Version
  * Keitai Denjuu Telefang: Speed Version
  * Kirby Family
  * Mario Family
  * Medarot: Kabuto Version
  * Medarot: Kuwagata Version
  * Medarot 2: Kabuto Version
  * Medarot 2: Kuwagata Version
  * Medarot 2: Parts Collection
  * Medarot 3: Kabuto Version
  * Medarot 3: Kuwagata Version
  * Medarot 4: Kabuto Version
  * Medarot 4: Kuwagata Version
  * Medarot 5: Susutake Mura no Tenkousei: Kabuto Version
  * Medarot 5: Susutake Mura no Tenkousei: Kuwagata Version
  * Ninja Gaiden Shadow/Shadow Warriors
  * Parts Collection: Medarot Kuma
  * Parts Collection 2
  * Pocket Shougi
  * Power Rangers: Lightspeed Rescue
  * Power Rangers: Time Force
  * Raku x Raku: Cut Shuu
  * Raku x Raku: Moji
  * Raku x Raku 3
  * Real Pro Yakyuu!: Central League Hen
  * Real Pro Yakyuu!: Pacific League Hen
  * Sewing Maching Operation Software
  * Spanky's Quest
  * Sylvanian Melody: Mori no Nakama to Odori Mashi!
  * Tail Gator
  * Tetris Plus
  * Tokoro San no Setagaya Country Club
  * WWF WrestleMania 2000

## To do:
  * Support for other versions of the sound engine (which are almost identical)
  * GBS file support
