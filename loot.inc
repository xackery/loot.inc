#Event AdvLootLeave "[MQ2] advloot #1# leave"
#Event AdvLootGive "[MQ2] advloot #1# give #2#"
#Event AdvLootEnable "[MQ2] advloot enable 1"
#Event AdvLootDisable "[MQ2] advloot disable 1"
#Event SetLoot "[MQ2] loot #1# #2#"
#Event GetBank "[MQ2] bank #1#"
#Event ParcelSent "#*#tells you, 'I will deliver#*#"
#Event IgnoreEmote "You may not loot this corpse at this time."

| /call LootMain in your code on the main loop
| You can add below it:
| /if (${Macro.Return.Length}) /echo LootMain: ${Macro.Return}
| if you want verbose debugging
Sub LootMain
	/if (!${Defined[LootIgnores]}) /declare LootIgnores string outer
	/if (!${Defined[LootIgnoreTimer]}) /declare LootIgnoreTimer timer outer
	/if (!${Defined[NinjaIniFile]}) /declare NinjaIniFile string outer Loot.ini
	/if (!${Defined[IsLootHudEnabled]}) /declare IsLootHudEnabled bool outer FALSE
	/if (!${Defined[IsLootEnabled]}) /declare IsLootEnabled bool outer FALSE
	/if (!${Defined[LastLoot]}) /declare LastLoot string outer
	/if (!${Defined[LastLootCount]}) /declare LastLootCount int outer
	/if (${Me.Casting.ID}) /return false
	/if (!${Defined[IsHideLootedEnabled]}) {
		/declare IsHideLootedEnabled bool outer true
		/if (${IsHideLootedEnabled}) /hidec looted
	}
	/if (!${IsLootEnabled}) /return loot is not enabled
	/if (${Me.CombatState.Equal[COMBAT]}) /return in combat
	/if (!${Me.FreeInventory}) /return no free inventory
	/if (${Cursor.ID}) /return item on cursor	
	/if (${Navigation.Active}) /return navigation is active
	/if (${Window[LootWnd].Open} && ${Corpse.Items} == 0) {
		/nomodkey /notify LootWnd LW_DoneButton leftmouseup
		/delay 1s !${Corpse.Open}
		/return loot inventory was open, closed it
	}

	/if (!${LootIgnoreTimer}) {
		/varset LootIgnores |
		/varset LootIgnoreTimer 120s
	}
	
	/declare i int local	
	/declare corpseID int local ${NearestCorpseID[]}
	/if (${corpseID} == 0) /return false
	/if (${Corpse.Open}) {
		/call LootCorpse
		/return ${Macro.Return}
	}
	
	/echo going to corpse \aw${Spawn[id ${corpseID}].Name}\ax to loot

:MoveToLoot	
	/if (${NearestSpawn[id ${corpseID}].Distance} > 10) {
		/if (!${Navigation.MeshLoaded}) /moveto loc ${Target.Y} ${Target.X} ${Target.Z}
		/if (${Navigation.MeshLoaded}) /nav spawn id ${corpseID}
		/doevents
		/delay 3s !${Navigation.Active} && !${Me.Moving}
		/doevents
		/if (${Target.ID} != ${corpseID}) /return false
		/doevents
		/if (!${Navigation.Active} && !${Me.Moving}) /goto :MoveToLoot
	}
	/target id ${corpseID}
	/delay 5
	/loot
	/delay 2s ${Corpse.Open}
	/delay 2s ${Corpse.Items}
	/doevents

	/if (!${Window[LootWnd].Open}) /return false
	/if (!${Corpse.Items}) /return false
	/call LootCorpse
	/if (${Macro.Return.Length}) /return LootCorpse: ${Macro.Return}
/return

sub NearestCorpseID
	/declare i int local
	/declare j int local
	/declare isIgnored bool local
	/declare nearestMobDistance float local 10000.0
	/declare currentDistance float local
	/declare nearestMobID int local 0	
	/declare corpseID int local 0
	
	/for i 1 to ${SpawnCount[corpse radius 40 zradius 150]}	
		/if (${NearestSpawn[${i},corpse radius 40 zradius 150].Name.Find[_]} < 1) /continue
		/varset corpseID ${NearestSpawn[${i},corpse radius 40 zradius 150].ID}	
		/varset isIgnored false
		/for j 1 to ${Math.Calc[${LootIgnores.Count[|]}+1]}
			/if (${LootIgnores.Token[${j},|]} != ${corpseID}) /continue
			/varset isIgnored true
		/next j
		/if (${isIgnored}) /continue
		/if (!${Navigation.MeshLoaded} && !${Navigation.PathExists[id ${corpseID}]}) /continue
		/varset currentDistance ${Navigation.PathLength[id ${corpseID}]}
		/if (${currentDistance} > ${nearestMobDistance}) /continue
		/varset nearestMobDistance ${currentDistance}
		/varset nearestMobID ${corpseID}
	/next i
/return ${nearestMobID}

sub LootCorpse
	/if (!${Window[LootWnd].Open}) /return false
	/if (!${Corpse.Items}) /return false
	/declare itemAction string local	
	/declare dropCount int local	
	/declare itemName string local
	/declare i int local
	/declare lootTotal int local ${Corpse.Items}
	/if (${lootTotal} == 0 && ${Corpse.Open}) {
		/nomodkey /notify LootWnd LW_DoneButton leftmouseup
		/delay 5 !${Corpse.Open}
		/return
	}
	
	/if (!${Corpse.Items}) /return false
	/for i 1 to ${lootTotal}
		/if (!${Window[LootWnd].Open}) /return false		
		/if (!${Corpse.Items}) /return false
		/if (!${Me.FreeInventory}) {
			/return no free inventory
		}
		/if (${Cursor.ID}) /return item on cursor
		/if (!${Bool[${Corpse.Item[${i}]}]}) /continue
		/varset itemName ${Corpse.Item[${i}].Name}
		/varcalc dropCount ${dropCount}+1

		/varset itemAction ${Ini[${NinjaIniFile},"${itemName.Left[1]}","${itemName}"]}
		/if (${itemAction.Equal[sell]}) /varset itemAction keep
		/if (${itemAction.Equal[guildbank]}) /varset itemAction keep
		/if (${itemAction.Find[bank|]}) /varset itemAction keep
		/if (${itemAction.Equal[personaltribute]}) /varset itemAction keep
		/if (${itemAction.Equal[guildtribute]}) /varset itemAction keep
		/if (${itemAction.Equal[vault]}) /varset itemAction keep
		/if (${itemAction.Find[parcel|]}) /varset itemAction keep
		/if (${itemAction.Equal[bazaar]}) /varset itemAction keep
		/if (${itemAction.Equal[ignore]}) /varset itemAction leave

		|/if (${AdvLoot.SList[${i}].Never}) /varset itemAction leave
		|/if (${AdvLoot.SList[${i}].Greed} || ${AdvLoot.SList[${i}].Need}) /varset itemAction keep
		
		/if (${itemAction.Equal[leave]}) {
			|/advloot shared "${itemName}" leave
			/echo leaving \at${itemName}\ax
			/continue
		}

		/if (${Corpse.Item[${i}].Lore} && ${FindItem[=${Corpse.Item[${i}]}].ID}) {
			/echo leaving lore item we already have \at${itemName}\ax
			/continue
		}

		/if (${itemAction.Equal[keep]}) {
			/call LootItem ${i} Keep right
			/varset LastLoot ${itemName}
			/echo looted \at${itemName}\ax
			/continue
		}

		/echo no action set for \at${itemName}\ax, default to auto loot
		/call LootItem ${i} Keep right
		/varset LastLoot ${itemName}
	/next i	

	/if (!${Corpse.Items}) /return false
	/if (${Corpse.Open}) {
		/nomodkey /notify LootWnd LW_DoneButton leftmouseup
		/delay 1s !${Corpse.Open}
	}
/return


Sub LootItem(int i,DoWhat,WhichButton)	
    /declare CorpseItemID int local ${Corpse.Item[${i}].ID}
    /nomodkey /shift /itemnotify loot${i} ${WhichButton}mouseup
    /delay 1s ${Window[ConfirmationDialogBox].Open} || !${Corpse.Item[${i}].NoDrop}
    /if (${Window[ConfirmationDialogBox].Open}) /nomodkey /notify ConfirmationDialogBox Yes_Button leftmouseup
    /delay 1s ${Cursor.ID} || ${WhichButton.NotEqual[left]}
    /if (${DoWhat.Equal[Destroy]} && ${Cursor.ID}==${CorpseItemID}) /destroy
    /delay 1s !${Corpse.Item[${i}].ID}	
/return

Sub Event_GetBank(line, itemName) 
	/declare itemCount int local -1

	/if (${itemName.Find[ ]} && ${Int[${itemName.Right[-${itemName.Find[ ]}]}]}) {		
		/varset itemCount ${Int[${itemName.Right[-${itemName.Find[ ]}]}]}
		/varset itemName ${itemName.Left[${Math.Calc[${itemName.Find[ ]}-1]}]}
	}
	/if (${itemCount}==0) /varset itemCount 1

	/call TakeFromBank "${itemName}" "${itemCount}"
	/if (${Macro.Return.Length}) {
		/echo failed to TakeFromBank: ${Macro.Return}
		/return
	}	
/return

Sub Event_SetLoot(line, itemAction, itemOption)
	/if (!${Defined[IsLootEnabled]}) /declare IsLootEnabled bool outer FALSE
	/if (${itemAction.Lower.Equal[parcel]}) /varset itemAction "${itemAction}|${itemOption}"
	/if (${itemAction.Lower.Equal[vault]}) /varset itemAction "${itemAction}|${itemOption}"
	/if (${itemAction.Lower.Equal[bank]}) /varset itemAction "${itemAction}|${itemOption}"
	/if (${itemAction.Lower.Equal[enable]}) {
		/varcalc IsLootEnabled TRUE	
		/echo auto looting is now TRUE
		/return
	}
	/if (${itemAction.Lower.Equal[disable]}) {
		/varcalc IsLootEnabled FALSE
		/echo auto looting is now FALSE
		/return
	}
	/if (${itemAction.Lower.Equal[begin]}) {
		/call DoLootStuff
		/return
	}
	/if (${itemAction.Lower.Equal[mode]}) {
		/loadhud loot
		/return
	}
	/if (${itemAction.Lower.Equal[sort]}) {
		/call SortItems
		/if (!${Macro.Return.Length}) /return
		/echo SortItems: ${Macro.Return}
		/return
	}
	/call EditIniLoot "${itemAction.Lower}"
	/if (${Macro.Return.Length}) /echo failed EditIniLoot: ${Macro.Return}
/return

Sub EditIniLoot(itemAction)
	/if (${Cursor.ID}==0) /return no item on cursor to ${itemAction}
	/if (!${itemAction.Equal[bazaar]} && !${itemAction.Equal[sell]} && !${itemAction.Find[vault]} && !${itemAction.Find[parcel]} && !${itemAction.Find[vault]} && !${itemAction.Find[bank]} && !${itemAction.Equal[personaltribute]} && !${itemAction.Equal[guildtribute]} && !${itemAction.Equal[ignore]}) /return unsupported action: ${itemAction}
	
	/echo -- Setting ${Cursor.Name} to ${itemAction}
	/ini "${NinjaIniFile}" ${Cursor.Name.Left[1]} "${Cursor.Name}" ${itemAction}
/return


Sub Event_AdvLootEnable(line, index) 
	/if (!${Defined[IsLootEnabled]}) /declare IsLootEnabled bool outer FALSE
	/varset IsLootEnabled TRUE
	/echo auto looting items is now enabled
/return

Sub Event_AdvLootDisable(line, index) 
	/if (!${Defined[IsLootEnabled]}) /declare IsLootEnabled bool outer FALSE
	/varset IsLootEnabled FALSE
	/echo auto looting items is now disabled
/return

Sub Event_AdvLootLeave(line, index) 
	/declare itemName string local
	/declare dropCount int local
	/declare npcName string local
	/if (!${Defined[i]}) /declare i int local 
	/varset i ${index}
	/varset itemName ${AdvLoot.SList[${i}]}
	/varset npcName ${AdvLoot.SList[${i}].Corpse.Name.Mid[1, ${AdvLoot.SList[${i}].Corpse.Name.Find['s_corpse]}-1]}
	/varset npcName ${npcName.Mid[1,${Math.Calc[${npcName.Length}-1]}]}
	/varset dropCount ${Ini[loot\${Zone.ShortName}.ini,${npcName},${AdvLoot.SList[${i}].ID}-DropCount]}
	/varcalc dropCount ${dropCount}+1
	/ini "loot\${Zone.ShortName}.ini" "${npcName}" "${AdvLoot.SList[${i}].ID}-DropCount" "${dropCount}"
	/advloot shared "${itemName}" leave			
	/delay 2s
	/if (${AdvLoot.LootInProgress}) /delay 2s
	/bc Left ${itemName} dropped by ${npcName} ${dropCount} times on corpse
/return

Sub EventAdv_LootGive(line, index, targetName) 
	/declare i int local
	/declare itemName string local
	/declare dropCount int local
	/declare npcName string local
	/varset i ${index}
	/varset itemName ${AdvLoot.SList[${i}]}
	/varset npcName ${AdvLoot.SList[${i}].Corpse.Name.Mid[1, ${AdvLoot.SList[${i}].Corpse.Name.Find['s_corpse]}-1]}
	/varset npcName ${npcName.Mid[1,${Math.Calc[${npcName.Length}-1]}]}
	/varset dropCount ${Ini[loot\${Zone.ShortName}.ini,${npcName},${AdvLoot.SList[${i}].ID}-DropCount]}
	/varcalc dropCount ${dropCount}+1
	/ini "loot\${Zone.ShortName}.ini" "${npcName}" "${AdvLoot.SList[${i}].ID}-DropCount" "${dropCount}"
	/advloot shared "${itemName}" giveto ${targetName} 1
	/delay 2s
	/if (${AdvLoot.LootInProgress}) /delay 2s
	/bc Gave ${itemName} dropped by ${npcName} ${dropCount} times to ${targetName}
/return

Sub DoLootStuff
	/keypress OPEN_INV_BAGS
	/declare someString string local

	/if (${someString.Equal[Words]}) /echo ok, good
	/varset someString Words
	/if (${someString.Equal[NULL]}) /echo ok, good

    /declare i int local
    /declare j int local
    /declare k int local
    /declare lootItem string
	/declare beforePlatinum int ${Me.Platinum}
	/echo starting loot stuff
	/declare itemAction string local


   | Top level inv has its own loop since ${FindItem[=${itemName}].InvSlot} looks at top lev inv slots first in Sub SellToVendor for the item to sell.
   | Loop through top level inventory slots for sellable items
   /for k 1 to 10
        /if (${InvSlot[pack${k}].Item.Container}>0) /continue
        /if (${InvSlot[pack${k}].Item.ID}==0) /continue

		/varset lootItem ${InvSlot[pack${k}].Item}
		/varset itemAction ${Ini[${NinjaIniFile},"${lootItem.Left[1]}","${lootItem}"]}
		/if (!${Bool[${itemAction}]}) /continue
		/if (${itemAction.Equal[sell]}) {
			/call SellToVendor "${lootItem}"
			/if (${Macro.Return.Length}) /echo failed to SellToVendor ${lootItem}: ${Macro.Return}
		}
		/if (${itemAction.Equal[guildbank]}) {
			/call DepositToGuildBank "${lootItem}"
			/if (${Macro.Return.Length}) /echo failed to DepositToGuildBank ${lootItem}: ${Macro.Return}
		}
		/if (${itemAction.Find[bank|]}) {
			/call DepositToBank "${lootItem}" "${itemAction}"
			/if (${Macro.Return.Length}) /echo failed to DepositToBank ${lootItem}: ${Macro.Return}
		}
		/if (${itemAction.Equal[personaltribute]}) {
			/call DonateToPersonalTribute "${lootItem}"
			/if (${Macro.Return.Length}) /echo failed to DonateToPersonalTribute ${lootItem}: ${Macro.Return}
		}
		/if (${itemAction.Equal[guildtribute]}) {
			/call DonateToGuildTribute "${lootItem}"
			/if (${Macro.Return.Length}) /echo failed to DonateToGuildTribute ${lootItem}: ${Macro.Return}
		}
		/if (${itemAction.Equal[vault]}) {
			/call DepositToVault "${lootItem}"
			/if (${Macro.Return.Length}) /echo failed to DepositToVault ${lootItem}: ${Macro.Return}
		}
		/if (${itemAction.Find[parcel|]}) {
			/call SendToParcel "${lootItem}" "${itemAction}"
			/if (${Macro.Return.Length}) /echo failed to SendToParcel ${lootItem}: ${Macro.Return}
		}
		/if (${itemAction.Equal[bazaar]}) {
			/call SellOnBazaar "${lootItem}"
			/if (${Macro.Return.Length}) /echo failed to SellOnBazaar ${lootItem}: ${Macro.Return}
		}
    /next k

    | Loop through top level inventory slots for containers
    /for i 1 to 10
        | Check if top level inv slot has a bag
        /if (${InvSlot[pack${i}].Item.Container}==0) /continue
        
		| Set j 1 to number of bag slots and loop through slots looking for items marked sell
        /for j 1 to ${InvSlot[pack${i}].Item.Container}
            /if (${InvSlot[pack${i}].Item.Item[${j}].ID}==0) /continue

            /varset lootItem ${InvSlot[pack${i}].Item.Item[${j}].Name}
			/varset itemAction ${Ini[${NinjaIniFile},"${lootItem.Left[1]}","${lootItem}"]}
			/if (!${Bool[${itemAction}]}) /continue
			/if (${itemAction.Equal[sell]}) {
				/call SellToVendor "${lootItem}"
				/if (${Macro.Return.Length}) /echo failed to SellToVendor ${lootItem}: ${Macro.Return}
			}
			/if (${itemAction.Equal[guildbank]}) {
				/call DepositToGuildBank "${lootItem}"
				/if (${Macro.Return.Length}) /echo failed to DepositToGuildBank ${lootItem}: ${Macro.Return}
			}
			/if (${itemAction.Find[bank|]}) {
				/call DepositToBank "${lootItem}" "${itemAction}"
				/if (${Macro.Return.Length}) /echo failed to DepositToBank ${lootItem}: ${Macro.Return}
			}
			/if (${itemAction.Equal[personaltribute]}) {
				/call DonateToPersonalTribute "${lootItem}"
				/if (${Macro.Return.Length}) /echo failed to DonateToPersonalTribute ${lootItem}: ${Macro.Return}
			}
			/if (${itemAction.Equal[guildtribute]}) {
				/call DonateToGuildTribute "${lootItem}"
				/if (${Macro.Return.Length}) /echo failed to DonateToGuildTribute ${lootItem}: ${Macro.Return}
			}
			/if (${itemAction.Equal[vault]}) {
				/call DepositToVault "${lootItem}"
				/if (${Macro.Return.Length}) /echo failed to DepositToVault ${lootItem}: ${Macro.Return}
			}
			/if (${itemAction.Find[parcel|]}) {
				/call SendToParcel "${lootItem}" "${itemAction}"
				/if (${Macro.Return.Length}) /echo failed to SendToParcel ${lootItem}: ${Macro.Return}
			}
			/if (${itemAction.Equal[bazaar]}) {
				/call SellOnBazaar "${lootItem}"
				/if (${Macro.Return.Length}) /echo failed to SellOnBazaar ${lootItem}: ${Macro.Return}
			}
        /next j
    /next i

    | Prevent spam from Sell event after selling items in Sub SellStuff
    /doevents flush Sell

	/keypress CLOSE_INV_BAGS
    /if (${Window[MerchantWnd].Open}) /nomodkey /notify MerchantWnd MW_Done_Button LeftMouseUp
    /if (${Window[guildbank].Open}) /keypress esc
	/if (${Window[BigBankWnd].Open}) /keypress esc
	/if (${Window[MerchantWnd].Open}) /nomodkey /notify MerchantWnd MW_Done_Button LeftMouseUp
	/if (${Window[TributeMasterWnd].Open}) /keypress esc
    
	/if (${Math.Calc[${Me.Platinum}-${beforePlatinum}]}>0) /echo ${Math.Calc[${Me.Platinum}-${beforePlatinum}]} total platinum earned
	
	/echo finished loot stuff
/return

Sub TakeFromBank(string itemName, int itemCount)
	/if (!${Window[BigBankWnd].Open}) /return bank window not open, open before running
	/declare i int local
	/declare j int local
	/declare slot string local
	/declare targetName string local
	/declare itemCollected int local 0

	/if (${itemCount}==0) /varset itemCount 1
	/if (${itemCount}<-1) /return invalid itemCount: ${itemCount}
	
	/if (${itemCount}==-1) {
		/echo looking for all ${itemName} in bank
	}
	/if (${itemCount}>0) {
		/echo looking for ${itemName} x${itemCount} in bank
	}
	
	/for i 1 to 24
		/varset slot bank${i}
		/if (${InvSlot[${slot}].Item.Container}) {
			/for j 1 to ${InvSlot[${slot}].Item.Container}
			  	/if (${InvSlot[${slot}].Item.Item[${j}].ID}==0) /continue			
				  
				/varset targetName ${InvSlot[${slot}].Item.Item[${j}].Name}
				/if (!${targetName.Upper.Equal[${itemName.Upper}]}) /continue
			
				/if (!${Window[${slot}].Open}) /itemnotify ${slot} rightmouseup
				
				/delay 5 ${Window[${slot}].Open}
				/if (!${Window[${slot}].Open}) /return open bag ${slot}

				/echo found ${targetName} at ${slot} ${j}
				
				/varcalc itemCollected ${itemCollected}+${InvSlot[${slot}].Item.Item[${j}].Stack}

				/delay 5 ${Cursor.ID}
				/nomodkey /shift /itemnotify in ${slot} ${j} leftmouseup				
				/delay 5
				/if (!${Cursor.ID}) /return item is not on cursor after grab
				
				/autoinv
				/delay 5
				/if (${Cursor.ID}) /return item is not off cursor after /autoinv

				/if (${itemCollected}>=${itemCount}) {
					/echo collected ${itemName} x ${itemCollected}
					/return true
				}
			/next j
			/if (${Window[bank${i}].Open}) /itemnotify bank${i} rightmouseup			
		}

        /if (${InvSlot[${slot}].Item.Container}>0) /continue
        /if (${InvSlot[${slot}].Item.ID}==0) /continue
		/varset targetName ${InvSlot[${slot}].Item}
	/next i

/return wanted ${itemName} x${itemCount}, but instead found x${itemCollected}


Sub DepositToBank(itemName, targetBank)
	/if (!${Bool[${targetBank}]}) /return
	/varset targetBank ${targetBank.Replace["bank|",""]}
	
	/if (!${Me.Name.Equal[${targetBank}]}) {
		/call SendToParcel "${itemName}" "${targetBank}"
		/if (${Macro.Return.Length}) /return SendToParcel: ${Macro.Return}
		/return
    }
    /if (!${Window[BigBankWnd].Open}) /return bank window not open
:deposit

	/if (${FindItem[=${itemName}].InvSlot}<=22) /return item not found
	/echo Depositing ${itemName}
	/if (!${InvSlot[pack${Float[${Math.Calc[${Select[${FindItem[=${itemName}].ItemSlot},22,23,24,25,26,27,28,29,30,31,32]}-1]}].Int}].Item.Container}) {
		/nomodkey /shift /itemnotify ${FindItem[=${itemName}].InvSlot} leftmouseup
	} else {
		/nomodkey /shift /itemnotify in Pack${Math.Calc[${FindItem[=${itemName}].ItemSlot}-22]} ${Math.Calc[${FindItem[=${itemName}].ItemSlot2}+1]} leftmouseup
	}
	/delay 5
	/nomodkey /notify BigBankWnd BIGB_AutoButton leftmouseup
	/delay 5
	/if (${FindItem[=${itemName}].InvSlot}) /goto :deposit
/return

Sub DepositToGuildBank(itemName)
    /if (!${Window[GuildBankWnd].Open}) /return bank window not open
:deposit
	/if (${Window[GuildBankWnd].Child[GBANK_DepositCountLabel].Text.Arg[2,:]}==0) /return guildbank is full
	/if (${FindItem[=${itemName}].InvSlot}<=22) /return invalid item invslot
	/echo Depositing ${itemName}
	/if (!${InvSlot[pack${Float[${Math.Calc[${Select[${FindItem[=${itemName}].ItemSlot},22,23,24,25,26,27,28,29,30,31,32]}-1]}].Int}].Item.Container}) {
		/nomodkey /shift /itemnotify ${FindItem[=${itemName}].InvSlot} leftmouseup
	} else {
		/nomodkey /shift /itemnotify in Pack${Math.Calc[${FindItem[=${itemName}].ItemSlot}-22]} ${Math.Calc[${FindItem[=${itemName}].ItemSlot2}+1]} leftmouseup
	}
	/delay 5
	/nomodkey /notify GuildBankWnd GBANK_DepositButton leftmouseup
	/delay 2s
	/if (${FindItem[=${itemName}].InvSlot}) /goto :deposit
/return

Sub SellToVendor(itemName)
	/if (!${Window[MerchantWnd].Open} && !${Window[MerchantWnd].Child[MW_MerchantName].Text.Find[Parcel]}) /return merchant not open
:sell
	/echo Selling ${itemName}
	/if (${InvSlot[pack${Select[${FindItem[=${itemName}].ItemSlot},23,24,25,26,27,28,29,30,31,32]}].Item.Container}) {
		/nomodkey /itemnotify in Pack${Math.Calc[${FindItem[=${itemName}].ItemSlot}-22]} ${Math.Calc[${FindItem[=${itemName}].ItemSlot2}+1]} leftmouseup
	} else {
		/nomodkey /itemnotify ${FindItem[=${itemName}].InvSlot} leftmouseup
	}
	/delay 5
	/nomodkey /shiftkey /notify merchantwnd MW_Sell_Button leftmouseup
	/doevents
	/delay 5
	/if (${FindItemCount[=${itemName}]}) /goto :sell
/return

Sub DonateToPersonalTribute(itemName)
    /if (!${Window[TributeMasterWnd].Open}) /return tribete window not open
	/if (!${Window[TributeMasterWnd].Text.Equal[Tribute Master]}) /return npc not a tribute master
:sell
	/echo donating ${itemName} to personal tribute
	/if (${InvSlot[pack${Select[${FindItem[=${itemName}].ItemSlot},23,24,25,26,27,28,29,30,31,32]}].Item.Container}) {
		/nomodkey /itemnotify in Pack${Math.Calc[${FindItem[=${itemName}].ItemSlot}-22]} ${Math.Calc[${FindItem[=${itemName}].ItemSlot2}+1]} leftmouseup
	} else {
		/nomodkey /itemnotify ${FindItem[=${itemName}].InvSlot} leftmouseup
	}
	/delay 5
	/nomodkey /shiftkey /notify TributeMasterWnd TMW_DonateButton leftmouseup
	/doevents
	/delay 2s
	/if (${FindItemCount[=${itemName}]}) /goto :sell
/return


Sub DonateToGuildTribute(itemName)
	/if (!${Window[TributeMasterWnd].Open}) /return tribute master not open
	/if (!${Window[TributeMasterWnd].Text.Equal[Guild Tribute Master]}) /return tribute master is not called guild tribute master
    /if (!${Window[MerchantWnd].Open}) /return merchant window not open
:sell
	/echo Selling ${itemName}
	/if (${InvSlot[pack${Select[${FindItem[=${itemName}].ItemSlot},23,24,25,26,27,28,29,30,31,32]}].Item.Container}) {
		/nomodkey /itemnotify in Pack${Math.Calc[${FindItem[=${itemName}].ItemSlot}-22]} ${Math.Calc[${FindItem[=${itemName}].ItemSlot2}+1]} leftmouseup
	} else {
		/nomodkey /itemnotify ${FindItem[=${itemName}].InvSlot} leftmouseup
	}
	/delay 5
	/nomodkey /shiftkey /notify TributeMasterWnd TMW_DonateButton leftmouseup
	/doevents
	/delay 2s
	/if (${FindItemCount[=${itemName}]}) /goto :sell
/return


Sub DepositToVault(itemName, targetVault)	
	/if (${Ini[${NinjaIniFile},Settings,VaultParcel].Length}) {
		/if (!${Me.Name.Equal[${Ini[${NinjaIniFile},Settings,VaultParcel]]}) {
		   /call SendToParcel "${itemName}" "${Ini[${NinjaIniFile},Settings,VaultParcel]}"
		   /if (${Macro.Return.Length}) /return SendToParcel: ${Macro.Return}
		   /return
	   }
    }
	/declare attemptCounter int local
	/declare isWindowOpen bool local
	/declare slot int local
	/declare pack int local
	/declare vaultIndex int local
	/declare taskStep int local
	/declare itemName string local
	/declare isItemInVault bool local FALSE

	/varset isWindowOpen ${Window[RealEstateItemsWnd].Open}
	/if (!${Window[RealEstateItemsWnd].Open}) {
		/windowstate RealEstateItemsWnd open
		/echo opening vault
		/delay 5
	}
	/if (!${Window[RealEstateItemsWnd].Open}) /return failed to open real estate window
	/for vaultIndex 1 to ${Window[RealEstateItemsWnd].Child[REIW_ItemList].Items}
		/varset itemName ${Window[RealEstateItemsWnd].Child[REIW_ItemList].List[${vaultIndex},2]}
		/if (${itemName.Length} > 0) /varset isItemInVault TRUE	
	/next vaultIndex
	/if (!${isItemInVault}) /return no existing item in vault
:nextItem
	/varset attemptCounter 0
	/varset pack ${InvSlot[pack${Select[${FindItem[=${itemName}].ItemSlot},23,24,25,26,27,28,29,30,31,32]}].Item.InvSlot.Pack}
	/varset slot ${FindItem[=${itemName}].InvSlot}
	/if (${FindItemCount[=${itemName}]}==0) /return no item to deposit	
:vault
	/varcalc attemptCounter ${attemptCounter}+1
	/if (${attemptCounter}>10) /return attempted too many times to insert
	/if (${Cursor.ID}) {
		/echo ${Cursor.Name} on cursor, /autoinv	
		/autoinv
		/delay 1s
	}
	
	/if (${InvSlot[pack${Select[${FindItem[=${itemName}].ItemSlot},23,24,25,26,27,28,29,30,31,32]}].Item.Container}) {
		/nomodkey /itemnotify in Pack${Math.Calc[${FindItem[=${itemName}].ItemSlot}-22]} ${Math.Calc[${FindItem[=${itemName}].ItemSlot2}+1]} leftmouseup
	} else {
		/nomodkey /itemnotify ${FindItem[=${itemName}].InvSlot} leftmouseup
	}

	/delay 5
	/nomodkey /shiftkey /notify RealEstateItemsWnd REIW_Move_Closet_Button leftmouseup
	/doevents
	/delay 2s
	/if (${FindItemCount[=${itemName}]}>0) /goto :nextItem
/return

Sub SendToParcel(itemName, targetPlayer)
	/if (!${Defined[targetPlayer]}) /return target player not defined
	/varset targetPlayer ${targetPlayer.Replace["parcel|",""]}
	/if (${targetPlayer.Equal[NULL]}) /return target player must be set
	/if (${targetPlayer.Equal[${Me.Name}]}) /return can't send items to yourself
    /if (!${Window[MerchantWnd].Open}) /return merchant window not open
	/if (!${Window[MerchantWnd].Child[MW_MerchantName].Text.Find[Parcel]}) /return merchant window is not parcel capable

	/notify MerchantWnd MW_MerchantSubWindows tabselect 3
	/delay 1s	
	/echo sending parcel ${itemName} to ${targetPlayer}
:send
	/if (${InvSlot[pack${Select[${FindItem[=${itemName}].ItemSlot},23,24,25,26,27,28,29,30,31,32]}].Item.Container}) {
		/nomodkey /itemnotify in Pack${Math.Calc[${FindItem[=${itemName}].ItemSlot}-22]} ${Math.Calc[${FindItem[=${itemName}].ItemSlot2}+1]} leftmouseup
	} else {
		/nomodkey /itemnotify ${FindItem[=${itemName}].InvSlot} leftmouseup
	}
	/nomodkey /shiftkey /notify MerchantWnd MW_Send_To_Edit leftmouseup
	/call ClearChat
	/call TypeIn "${targetPlayer}"
	/delay 5
	/nomodkey /shiftkey /notify MerchantWnd MW_Send_Button leftmouseup
	/if (${FindItemCount[=${itemName}]}) /goto :send
/return

Sub TypeIn(message) 
	/declare char string local 
	/declare i int local 
	/for i 1 to ${message.Length} 
		/varset char ${message.Mid[${i},1]} 
		/if (!${char.Length}) { 
			/nomodkey /keypress Space chat 
		} else { 
			/if (${char.Left[1].Equal[(]}) /return
			/if (${char.Left[1].Equal[)]}) /return
			/nomodkey /keypress ${char} chat 
		} 
	/next i
/return


Sub SellOnBazaar(itemName, targetPlayer)
	/return not implemented
/return

Sub ClearChat
:Try1
	/ctrlkey /shiftkey /keypress home chat
	/delay 5
	/ctrlkey /shiftkey /keypress delete chat
	/delay 5
	/if (${Window[MerchantWnd].Child[MW_Send_To_Edit].Text.Length}>0) /goto :Try1
/return

sub Event_Sort
	/call SortItems
	/if (${Macro.Return.Length}) /echo failed SortItems: ${Macro.Return}
/return

sub SortItems
	/declare i int local
	/declare j int local
	/declare freeSlot string local
	/declare lootItem string local
	/declare lootAction string local
	/declare bagID string local
	/declare slotID int local
	/declare itemAction string local
	/if (${Cursor.ID}) /return take item off cursor first
	/declare sortCount int local
	/declare firstBagID int local
	/declare firstSlotID int local

	/varset freeSlot ${FirstFreeSlot[]}
	/if (!${freeSlot.Length}) /return no first free slot found
	/if (${freeSlot.Count[|]}==0) /return no pipe found in free slot filter		
	/varset firstBagID ${freeSlot.Token[1,|]}
	/varset firstSlotID ${freeSlot.Token[2,|]}

    /for i 1 to 10
        /if (${InvSlot[pack${i}].Item.Container}==0) /continue
        |/if (!${Window[${i}].Open}) /itemnotify ${slot} rightmouseup
		|/delay 5 ${Window[${i}].Open}
		|if (!${Window[${i}].Open}) /return open bag ${i}
		
		| Set j 1 to number of bag slots and loop through slots looking for items marked sell
        /for j 1 to ${InvSlot[pack${i}].Item.Container}
			/if (${Int[${firstBagID}]} <= ${i} && ${Int[${firstSlotID}]} <= ${j}) /return sorted ${sortCount} items
            /if (${InvSlot[pack${i}].Item.Item[${j}].ID}==0) /continue
			/varset freeSlot ${LastFreeSlot[]}
			/if (!${freeSlot.Length}) /return no last free slots found
			/if (${freeSlot.Count[|]}==0) /return no pipe found in filter		
			/varset bagID ${freeSlot.Token[1,|]}
			/varset slotID ${freeSlot.Token[2,|]}

	        /varset lootItem ${InvSlot[pack${i}].Item.Item[${j}].Name}
			/varset itemAction ${Ini[${NinjaIniFile},"${lootItem.Left[1]}","${lootItem}"]}
			/if (!${Bool[${itemAction}]}) /continue
			/varcalc sortCount ${sortCount}+1
			/nomodkey /shift /itemnotify in pack${i} ${j} leftmouseup
			/delay 5 ${Cursor.ID}
			/delay 5
			
			/if (!${Cursor.ID}) /return pick up ${lootItem} failed
			/nomodkey /shift /itemnotify in pack${bagID} ${slotID} leftmouseup
			/delay 5 !${Cursor.ID}
			/if (${Cursor.ID}) /return drop ${lootItem} failed
        /next j
    /next i
	/echo sorted ${sortCount} items
/return


sub FirstFreeSlot
	/declare i int local
	/declare j int local
	
    /for i 1 to 10
        /if (${InvSlot[pack${i}].Item.Container}==0) /continue
        /for j 1 to ${InvSlot[pack${i}].Item.Container}
            /if (${InvSlot[pack${i}].Item.Item[${j}].ID}==0) /return ${i}|${j}
        /next j
    /next i
/return

sub LastFreeSlot
	/declare i int local
	/declare j int local
	
    /for i 10 downto 1
        /if (${InvSlot[pack${i}].Item.Container}==0) /continue
        /for j ${InvSlot[pack${i}].Item.Container} downto 1
            /if (${InvSlot[pack${i}].Item.Item[${j}].ID}==0) /return ${i}|${j}
        /next j
    /next i
/return

sub Event_IgnoreEmote
	/if (!${Target.Type.Equal[Corpse]}) /return
	/varset LootIgnores ${LootIgnores}|${Target.ID}	
	/echo ignoring \aw${Target.Name}\ax
	/target clear
/return