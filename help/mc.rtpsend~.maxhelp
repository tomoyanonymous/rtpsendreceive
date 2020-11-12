{
	"patcher" : 	{
		"fileversion" : 1,
		"appversion" : 		{
			"major" : 8,
			"minor" : 1,
			"revision" : 5,
			"architecture" : "x64",
			"modernui" : 1
		}
,
		"classnamespace" : "box",
		"openrect" : [ 174.0, 125.0, 853.0, 749.0 ],
		"bglocked" : 0,
		"openinpresentation" : 0,
		"default_fontsize" : 12.0,
		"default_fontface" : 0,
		"default_fontname" : "Arial",
		"gridonopen" : 1,
		"gridsize" : [ 15.0, 15.0 ],
		"gridsnaponopen" : 1,
		"objectsnaponopen" : 1,
		"statusbarvisible" : 2,
		"toolbarvisible" : 1,
		"lefttoolbarpinned" : 0,
		"toptoolbarpinned" : 0,
		"righttoolbarpinned" : 0,
		"bottomtoolbarpinned" : 0,
		"toolbars_unpinned_last_save" : 0,
		"tallnewobj" : 0,
		"boxanimatetime" : 200,
		"enablehscroll" : 1,
		"enablevscroll" : 1,
		"devicewidth" : 853.0,
		"description" : "Transmit mc audio signal over IP network using rtp protocol. An internal sample format is an uncompressed PCM, not floating-point but a 16bit integer. The rtp packets are send on UDP protocol. User can choose way of initializing connection from either raw RTP (No handshake) or RTSP (handshake with TCP).",
		"digest" : "send audio stream via rtp protocol",
		"tags" : "",
		"style" : "",
		"subpatcher_template" : "",
		"showrootpatcherontab" : 0,
		"showontab" : 0,
		"assistshowspatchername" : 0,
		"boxes" : [ 			{
				"box" : 				{
					"id" : "obj-9",
					"maxclass" : "attrui",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 295.0, 115.0, 150.0, 22.0 ]
				}

			}
, 			{
				"box" : 				{
					"fontname" : "Arial",
					"fontsize" : 13.0,
					"id" : "obj-1",
					"maxclass" : "newobj",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 295.0, 155.0, 174.0, 23.0 ],
					"saved_object_attributes" : 					{
						"filename" : "rtphelpstarter.js",
						"parameter_enable" : 0
					}
,
					"text" : "js rtphelpstarter mc.rtpsend~"
				}

			}
, 			{
				"box" : 				{
					"fontname" : "Arial",
					"fontsize" : 13.0,
					"id" : "obj-5",
					"maxclass" : "newobj",
					"numinlets" : 0,
					"numoutlets" : 0,
					"patcher" : 					{
						"fileversion" : 1,
						"appversion" : 						{
							"major" : 8,
							"minor" : 1,
							"revision" : 5,
							"architecture" : "x64",
							"modernui" : 1
						}
,
						"classnamespace" : "box",
						"rect" : [ 0.0, 26.0, 853.0, 723.0 ],
						"bglocked" : 0,
						"openinpresentation" : 0,
						"default_fontsize" : 13.0,
						"default_fontface" : 0,
						"default_fontname" : "Arial",
						"gridonopen" : 1,
						"gridsize" : [ 15.0, 15.0 ],
						"gridsnaponopen" : 1,
						"objectsnaponopen" : 1,
						"statusbarvisible" : 2,
						"toolbarvisible" : 1,
						"lefttoolbarpinned" : 0,
						"toptoolbarpinned" : 0,
						"righttoolbarpinned" : 0,
						"bottomtoolbarpinned" : 0,
						"toolbars_unpinned_last_save" : 0,
						"tallnewobj" : 0,
						"boxanimatetime" : 200,
						"enablehscroll" : 1,
						"enablevscroll" : 1,
						"devicewidth" : 0.0,
						"description" : "",
						"digest" : "",
						"tags" : "",
						"style" : "",
						"subpatcher_template" : "",
						"showontab" : 1,
						"assistshowspatchername" : 0,
						"boxes" : [  ],
						"lines" : [  ]
					}
,
					"patching_rect" : [ 106.0, 88.0, 50.0, 23.0 ],
					"saved_object_attributes" : 					{
						"description" : "",
						"digest" : "",
						"fontsize" : 13.0,
						"globalpatchername" : "",
						"tags" : ""
					}
,
					"text" : "p ?",
					"varname" : "q_tab"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-2",
					"maxclass" : "newobj",
					"numinlets" : 0,
					"numoutlets" : 0,
					"patcher" : 					{
						"fileversion" : 1,
						"appversion" : 						{
							"major" : 8,
							"minor" : 1,
							"revision" : 5,
							"architecture" : "x64",
							"modernui" : 1
						}
,
						"classnamespace" : "box",
						"rect" : [ 174.0, 151.0, 853.0, 723.0 ],
						"bglocked" : 0,
						"openinpresentation" : 0,
						"default_fontsize" : 12.0,
						"default_fontface" : 0,
						"default_fontname" : "Arial",
						"gridonopen" : 1,
						"gridsize" : [ 15.0, 15.0 ],
						"gridsnaponopen" : 1,
						"objectsnaponopen" : 1,
						"statusbarvisible" : 2,
						"toolbarvisible" : 1,
						"lefttoolbarpinned" : 0,
						"toptoolbarpinned" : 0,
						"righttoolbarpinned" : 0,
						"bottomtoolbarpinned" : 0,
						"toolbars_unpinned_last_save" : 0,
						"tallnewobj" : 0,
						"boxanimatetime" : 200,
						"enablehscroll" : 1,
						"enablevscroll" : 1,
						"devicewidth" : 0.0,
						"description" : "",
						"digest" : "",
						"tags" : "",
						"style" : "",
						"subpatcher_template" : "",
						"showontab" : 1,
						"isolateaudio" : 1,
						"assistshowspatchername" : 0,
						"boxes" : [ 							{
								"box" : 								{
									"fontname" : "Lato",
									"fontsize" : 13.0,
									"id" : "obj-3",
									"maxclass" : "comment",
									"numinlets" : 1,
									"numoutlets" : 0,
									"patching_rect" : [ 10.0, 58.540092766284943, 585.0, 22.0 ],
									"text" : "send audio stream via rtp protocol",
									"textcolor" : [ 0.0, 0.0, 0.0, 1.0 ]
								}

							}
, 							{
								"box" : 								{
									"id" : "obj-37",
									"maxclass" : "scope~",
									"numinlets" : 2,
									"numoutlets" : 0,
									"patching_rect" : [ 30.834952294826508, 589.330088555812836, 346.0, 113.495145857334137 ]
								}

							}
, 							{
								"box" : 								{
									"id" : "obj-36",
									"maxclass" : "newobj",
									"numinlets" : 1,
									"numoutlets" : 1,
									"outlettype" : [ "bang" ],
									"patching_rect" : [ 30.834952294826508, 178.912618577480316, 58.0, 22.0 ],
									"text" : "loadbang"
								}

							}
, 							{
								"box" : 								{
									"id" : "obj-35",
									"maxclass" : "message",
									"numinlets" : 2,
									"numoutlets" : 1,
									"outlettype" : [ "" ],
									"patching_rect" : [ 30.834952294826508, 216.504851400852203, 128.0, 22.0 ],
									"text" : "increment 1000. 1000."
								}

							}
, 							{
								"box" : 								{
									"id" : "obj-32",
									"maxclass" : "newobj",
									"numinlets" : 1,
									"numoutlets" : 2,
									"outlettype" : [ "multichannelsignal", "" ],
									"patching_rect" : [ 30.834952294826508, 545.388345122337341, 346.0, 22.0 ],
									"text" : "mc.rtpreceive~ @address 127.0.0.1 @port 30000 @channels 8"
								}

							}
, 							{
								"box" : 								{
									"bubble" : 1,
									"fontname" : "Arial",
									"fontsize" : 13.0,
									"id" : "obj-31",
									"linecount" : 3,
									"maxclass" : "comment",
									"numinlets" : 1,
									"numoutlets" : 0,
									"patching_rect" : [ 660.378758460283279, 389.825237154960632, 162.0, 54.0 ],
									"text" : "Internal buffer size relative to signal vector size"
								}

							}
, 							{
								"box" : 								{
									"attr" : "ringbuf_framenum",
									"id" : "obj-30",
									"maxclass" : "attrui",
									"numinlets" : 1,
									"numoutlets" : 1,
									"outlettype" : [ "" ],
									"patching_rect" : [ 508.378758460283279, 405.825237154960632, 150.0, 22.0 ],
									"text_width" : 112.980456322431564
								}

							}
, 							{
								"box" : 								{
									"bubble" : 1,
									"fontname" : "Arial",
									"fontsize" : 13.0,
									"id" : "obj-29",
									"linecount" : 2,
									"maxclass" : "comment",
									"numinlets" : 1,
									"numoutlets" : 0,
									"patching_rect" : [ 660.378758460283279, 319.412617206573486, 171.349513351917267, 40.0 ],
									"text" : "Acceptable number of packets for reordering"
								}

							}
, 							{
								"box" : 								{
									"attr" : "reorder_queue_size",
									"id" : "obj-27",
									"maxclass" : "attrui",
									"numinlets" : 1,
									"numoutlets" : 1,
									"outlettype" : [ "" ],
									"patching_rect" : [ 508.378758460283279, 328.412617206573486, 150.0, 22.0 ],
									"text_width" : 112.980456322431564
								}

							}
, 							{
								"box" : 								{
									"bubble" : 1,
									"fontname" : "Arial",
									"fontsize" : 13.0,
									"id" : "obj-26",
									"linecount" : 3,
									"maxclass" : "comment",
									"numinlets" : 1,
									"numoutlets" : 0,
									"patching_rect" : [ 660.378758460283279, 256.752423644065857, 171.349513351917267, 54.0 ],
									"text" : "Available Port Range for Audio Transmittion in RTSP mode"
								}

							}
, 							{
								"box" : 								{
									"bubble" : 1,
									"fontname" : "Arial",
									"fontsize" : 13.0,
									"id" : "obj-25",
									"linecount" : 2,
									"maxclass" : "comment",
									"numinlets" : 1,
									"numoutlets" : 0,
									"patching_rect" : [ 313.165046334266663, 396.825237154960632, 177.174755990505219, 40.0 ],
									"text" : "Connection Retry Interval (in ms)"
								}

							}
, 							{
								"box" : 								{
									"bubble" : 1,
									"fontname" : "Arial",
									"fontsize" : 13.0,
									"id" : "obj-24",
									"maxclass" : "comment",
									"numinlets" : 1,
									"numoutlets" : 0,
									"patching_rect" : [ 313.165046334266663, 367.247568130493164, 177.174755990505219, 25.0 ],
									"text" : "Use RTSP for handshake"
								}

							}
, 							{
								"box" : 								{
									"bubble" : 1,
									"fontname" : "Arial",
									"fontsize" : 13.0,
									"id" : "obj-23",
									"maxclass" : "comment",
									"numinlets" : 1,
									"numoutlets" : 0,
									"patching_rect" : [ 313.165046334266663, 328.412617206573486, 177.174755990505219, 25.0 ],
									"text" : " Audio Channels"
								}

							}
, 							{
								"box" : 								{
									"bubble" : 1,
									"fontname" : "Arial",
									"fontsize" : 13.0,
									"id" : "obj-22",
									"maxclass" : "comment",
									"numinlets" : 1,
									"numoutlets" : 0,
									"patching_rect" : [ 313.165046334266663, 290.548540055751801, 177.174755990505219, 25.0 ],
									"text" : "Destination Main Port"
								}

							}
, 							{
								"box" : 								{
									"bubble" : 1,
									"fontname" : "Arial",
									"fontsize" : 13.0,
									"id" : "obj-28",
									"maxclass" : "comment",
									"numinlets" : 1,
									"numoutlets" : 0,
									"patching_rect" : [ 313.165046334266663, 256.752423644065857, 177.174755990505219, 25.0 ],
									"text" : "Destination IP Address"
								}

							}
, 							{
								"box" : 								{
									"attr" : "max_port",
									"id" : "obj-21",
									"maxclass" : "attrui",
									"numinlets" : 1,
									"numoutlets" : 1,
									"outlettype" : [ "" ],
									"patching_rect" : [ 508.378758460283279, 290.548540055751801, 150.0, 22.0 ],
									"text_width" : 72.815532982349396
								}

							}
, 							{
								"box" : 								{
									"attr" : "min_port",
									"id" : "obj-18",
									"maxclass" : "attrui",
									"numinlets" : 1,
									"numoutlets" : 1,
									"outlettype" : [ "" ],
									"patching_rect" : [ 508.378758460283279, 256.752423644065857, 150.0, 22.0 ],
									"text_width" : 72.815532982349396
								}

							}
, 							{
								"box" : 								{
									"attr" : "retry_rate",
									"id" : "obj-19",
									"maxclass" : "attrui",
									"numinlets" : 1,
									"numoutlets" : 1,
									"outlettype" : [ "" ],
									"patching_rect" : [ 161.165046334266663, 405.825237154960632, 150.0, 22.0 ],
									"text_width" : 72.815532982349396
								}

							}
, 							{
								"box" : 								{
									"attr" : "use_rtsp",
									"id" : "obj-20",
									"maxclass" : "attrui",
									"numinlets" : 1,
									"numoutlets" : 1,
									"outlettype" : [ "" ],
									"patching_rect" : [ 161.165046334266663, 369.90290755033493, 150.0, 22.0 ],
									"text_width" : 72.815532982349396
								}

							}
, 							{
								"box" : 								{
									"attr" : "channels",
									"id" : "obj-17",
									"maxclass" : "attrui",
									"numinlets" : 1,
									"numoutlets" : 1,
									"outlettype" : [ "" ],
									"patching_rect" : [ 161.165046334266663, 330.097082853317261, 150.0, 22.0 ],
									"text_width" : 72.815532982349396
								}

							}
, 							{
								"box" : 								{
									"attr" : "port",
									"id" : "obj-16",
									"maxclass" : "attrui",
									"numinlets" : 1,
									"numoutlets" : 1,
									"outlettype" : [ "" ],
									"patching_rect" : [ 161.165046334266663, 294.174753248691559, 150.0, 22.0 ],
									"text_width" : 72.815532982349396
								}

							}
, 							{
								"box" : 								{
									"attr" : "address",
									"id" : "obj-15",
									"maxclass" : "attrui",
									"numinlets" : 1,
									"numoutlets" : 1,
									"outlettype" : [ "" ],
									"patching_rect" : [ 161.165046334266663, 258.252423644065857, 150.0, 22.0 ],
									"text_width" : 72.815532982349396
								}

							}
, 							{
								"box" : 								{
									"fontname" : "Lato",
									"fontsize" : 13.0,
									"id" : "obj-13",
									"maxclass" : "comment",
									"numinlets" : 1,
									"numoutlets" : 0,
									"patching_rect" : [ 518.602054506540298, 680.825234413146973, 313.126217305660248, 22.0 ],
									"text" : "https://github.com/tomoyanonymous/rtpsendreceive",
									"textcolor" : [ 0.0, 0.0, 0.0, 1.0 ],
									"textjustification" : 2
								}

							}
, 							{
								"box" : 								{
									"fontname" : "Lato",
									"fontsize" : 13.0,
									"id" : "obj-12",
									"maxclass" : "comment",
									"numinlets" : 1,
									"numoutlets" : 0,
									"patching_rect" : [ 626.369043320417404, 656.825234413146973, 205.359228491783142, 22.0 ],
									"text" : "Made by Tomoya Matsuura",
									"textcolor" : [ 0.0, 0.0, 0.0, 1.0 ],
									"textjustification" : 2
								}

							}
, 							{
								"box" : 								{
									"id" : "obj-11",
									"maxclass" : "newobj",
									"numinlets" : 2,
									"numoutlets" : 1,
									"outlettype" : [ "multichannelsignal" ],
									"patching_rect" : [ 30.834952294826508, 258.252423644065857, 120.0, 22.0 ],
									"text" : "mc.cycle~ @chans 8"
								}

							}
, 							{
								"box" : 								{
									"fontname" : "Lato",
									"fontsize" : 13.0,
									"id" : "obj-10",
									"linecount" : 4,
									"maxclass" : "comment",
									"numinlets" : 1,
									"numoutlets" : 0,
									"patching_rect" : [ 10.320512861013412, 82.540092766284943, 585.0, 69.0 ],
									"text" : "Transmit mc audio signal over IP network using rtp protocol. An internal sample format is an uncompressed PCM, not floating-point but a 16bit integer.\nThe rtp packets are send on UDP protocol. User can choose way of initializing connection from either raw RTP (No handshake) or RTSP (handshake with TCP).",
									"textcolor" : [ 0.490196078431373, 0.498039215686275, 0.517647058823529, 1.0 ]
								}

							}
, 							{
								"box" : 								{
									"fontname" : "Lato",
									"fontsize" : 13.0,
									"id" : "obj-9",
									"linecount" : 7,
									"maxclass" : "comment",
									"numinlets" : 1,
									"numoutlets" : 0,
									"patching_rect" : [ 533.165161103010178, 491.019413828849792, 298.563110709190369, 116.0 ],
									"text" : "Note for port assignment:\nOne more port next to main port will be used for RTCP protocol.\nIn RTSP mode, initial handshake is done via main port and a port for audio transmission will be choosen automatically. User can limit the port by min_port and max_port attribute.",
									"textcolor" : [ 0.0, 0.0, 0.0, 1.0 ]
								}

							}
, 							{
								"box" : 								{
									"fontname" : "Lato",
									"fontsize" : 48.0,
									"id" : "obj-5",
									"maxclass" : "comment",
									"numinlets" : 1,
									"numoutlets" : 0,
									"patching_rect" : [ 10.0, 0.0, 275.941748917102814, 64.0 ],
									"text" : "mc.rtpsend~"
								}

							}
, 							{
								"box" : 								{
									"id" : "obj-1",
									"maxclass" : "newobj",
									"numinlets" : 1,
									"numoutlets" : 0,
									"patching_rect" : [ 30.834952294826508, 491.019413828849792, 333.0, 22.0 ],
									"text" : "mc.rtpsend~ @address 127.0.0.1 @port 30000 @channels 8"
								}

							}
 ],
						"lines" : [ 							{
								"patchline" : 								{
									"destination" : [ "obj-1", 0 ],
									"source" : [ "obj-11", 0 ]
								}

							}
, 							{
								"patchline" : 								{
									"destination" : [ "obj-1", 0 ],
									"midpoints" : [ 170.665046334266663, 289.349513351917267, 40.334952294826508, 289.349513351917267 ],
									"source" : [ "obj-15", 0 ]
								}

							}
, 							{
								"patchline" : 								{
									"destination" : [ "obj-1", 0 ],
									"midpoints" : [ 170.665046334266663, 325.349513351917267, 40.334952294826508, 325.349513351917267 ],
									"source" : [ "obj-16", 0 ]
								}

							}
, 							{
								"patchline" : 								{
									"destination" : [ "obj-1", 0 ],
									"midpoints" : [ 170.665046334266663, 365.233008444309235, 40.334952294826508, 365.233008444309235 ],
									"source" : [ "obj-17", 0 ]
								}

							}
, 							{
								"patchline" : 								{
									"destination" : [ "obj-1", 0 ],
									"midpoints" : [ 517.878758460283279, 281.233008444309235, 494.815534353256226, 281.233008444309235, 494.815534353256226, 467.281553983688354, 40.334952294826508, 467.281553983688354 ],
									"source" : [ "obj-18", 0 ]
								}

							}
, 							{
								"patchline" : 								{
									"destination" : [ "obj-1", 0 ],
									"midpoints" : [ 170.665046334266663, 434.233008444309235, 40.334952294826508, 434.233008444309235 ],
									"source" : [ "obj-19", 0 ]
								}

							}
, 							{
								"patchline" : 								{
									"destination" : [ "obj-1", 0 ],
									"midpoints" : [ 170.665046334266663, 398.233008444309235, 40.334952294826508, 398.233008444309235 ],
									"source" : [ "obj-20", 0 ]
								}

							}
, 							{
								"patchline" : 								{
									"destination" : [ "obj-1", 0 ],
									"midpoints" : [ 517.878758460283279, 322.970872402191162, 494.815534353256226, 322.970872402191162, 494.815534353256226, 465.339806437492371, 40.334952294826508, 465.339806437492371 ],
									"source" : [ "obj-21", 0 ]
								}

							}
, 							{
								"patchline" : 								{
									"destination" : [ "obj-1", 0 ],
									"midpoints" : [ 517.878758460283279, 363.106795251369476, 494.815534353256226, 363.106795251369476, 494.815534353256226, 467.281553983688354, 40.334952294826508, 467.281553983688354 ],
									"source" : [ "obj-27", 0 ]
								}

							}
, 							{
								"patchline" : 								{
									"destination" : [ "obj-1", 0 ],
									"midpoints" : [ 517.878758460283279, 467.281553983688354, 40.334952294826508, 467.281553983688354 ],
									"source" : [ "obj-30", 0 ]
								}

							}
, 							{
								"patchline" : 								{
									"destination" : [ "obj-37", 0 ],
									"source" : [ "obj-32", 0 ]
								}

							}
, 							{
								"patchline" : 								{
									"destination" : [ "obj-11", 0 ],
									"source" : [ "obj-35", 0 ]
								}

							}
, 							{
								"patchline" : 								{
									"destination" : [ "obj-35", 0 ],
									"source" : [ "obj-36", 0 ]
								}

							}
 ]
					}
,
					"patching_rect" : [ 18.0, 87.5, 47.0, 22.0 ],
					"saved_object_attributes" : 					{
						"description" : "",
						"digest" : "",
						"globalpatchername" : "",
						"tags" : ""
					}
,
					"text" : "p basic"
				}

			}
 ],
		"lines" : [ 			{
				"patchline" : 				{
					"destination" : [ "obj-1", 0 ],
					"source" : [ "obj-9", 0 ]
				}

			}
 ],
		"dependency_cache" : [ 			{
				"name" : "rtphelpstarter.js",
				"bootpath" : "~/codes/rtpsendreceive/help",
				"patcherrelativepath" : ".",
				"type" : "TEXT",
				"implicit" : 1
			}
, 			{
				"name" : "mc.rtpsend~.mxo",
				"type" : "iLaX"
			}
, 			{
				"name" : "mc.rtpreceive~.mxo",
				"type" : "iLaX"
			}
 ],
		"autosave" : 0
	}

}
