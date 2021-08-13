# loot.inc
Works on eqemu servers. Star if you like this!

Autoloot stuff:

![loot](https://media.giphy.com/media/mC2DBfdm8HWex6oyk1/giphy.gif)

Item on Cursor sorting:

![sort](https://media.giphy.com/media/gBJ8cKgtVfr6o3ZBcR/giphy.gif)


# Setup
- Download loot.inc, put it in your Macros directory
- On the main loop macro file, add `#include loot.inc` to the top most area
- Add to your main loop of a macro, add this line `/call LootMain`
- Create a hotbutton panel, I usually set it to shift+4, and add the following hotbuttons:
```sh
# This begins selling or banking when a banker or merchant window is open
/hotbutton Begin /echo loot begin 1
# While holding an item, click this button to mark it to sell
/hotbutton Sell /echo loot sell 1
# While holding an item, click this button to mark it to ignore
/hotbutton Ignore /echo loot ignore 1
# While holding an item, click this button to mark it to put in bank
/hotbutton Bank /echo loot bank playerName
# While holding an item, click this button to mark it to go to guild bank
/hotbutton "Guild Bank" /echo loot guildbank
# While holding an item, click this button to mark it be personal tribute
/hotbutton PTribute /echo loot personaltribute 1
# While holding an item, click this button to mark it to guild tribute
/hotbutton GTribute /echo loot guildtribute 1
# While holding an item, click this button to mark it to keep
/hotbutton Sell /echo loot keep 1
# While holding an item, click this button to mark it to go to another person's inventory. (Not yet implemented)
/hotbutton ParcelPlayerName /echo loot parcel PlayerName
```
- In another area, add these hotkeys:
```sh
# turn on auto looting
/hotbutton AutoLootON /echo advloot enable 1
# turn off auto looting
/hotbutton AutoLootOFF /echo advloot disable 1
```
- in your MQ2HUD.ini, add this entry:
```
[Loot]
LastMouseOverText=7,2,50,0,255,0,${If[${Cursor.ID},${Ini[loot.ini,"${Cursor.Name.Left[1]}","${Cursor.Name}"]},]}
```
- in game, type /loadhud loot
- in game, type /hud normal
- test by picking up an item, and clicking e.g. the ignore button, and see if a green ignore text appears like soL
![ignore](https://i.imgur.com/AiaG1KO.png)

# Usage
- Start your macro, and press the AutoLootOn hotbutton.
- Pick up items as they go into your bag, and flag them accoringly.
- Edit your Loot.ini to tweak any settings

# Changelog
- 2021-08-13 initial version