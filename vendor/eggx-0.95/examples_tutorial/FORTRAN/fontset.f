*
      integer win,key
      integer status

      call gopen(140,80,win)

      call newfontset(win,'-*-fixed-medium-r-normal--16-*'//CHAR(0),
     &                 status)
      call drawstr(win, 16.0, 32.0, 0.0,
     &             'ギコハハハ…!!', 0.0, 14 )
      call drawstr(win, 16.0, 48.0, 0.0,
     &             'ギコハにゃーん'//CHAR(0), 0.0, -1 )
      call ggetch(key)

      call gclose(win)

      end
*
