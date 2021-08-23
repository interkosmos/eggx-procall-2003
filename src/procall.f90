! procall.f90
!
! Fortran 2003 interfaces to the FORTRAN 77 functions and routines in the
! EGGX/ProCALL graphics library. For more information on EGGX/ProCALL, see:
!
!   https://www.ir.isas.jaxa.jp/~cyamauch/eggx_procall/
!
! Author:  Philipp Engel
! Licence: ISC
module procall
    implicit none
    private

    ! DS9-compatible colour patterns.
    integer, parameter, public :: PROCALL_DS9_GRAY                     = 0
    integer, parameter, public :: PROCALL_DS9_RED                      = 1
    integer, parameter, public :: PROCALL_DS9_GREEN                    = 2
    integer, parameter, public :: PROCALL_DS9_BLUE                     = 3
    integer, parameter, public :: PROCALL_DS9_A                        = 4
    integer, parameter, public :: PROCALL_DS9_B                        = 5
    integer, parameter, public :: PROCALL_DS9_BB                       = 6
    integer, parameter, public :: PROCALL_DS9_HE                       = 7
    integer, parameter, public :: PROCALL_DS9_I8                       = 8
    integer, parameter, public :: PROCALL_DS9_AIPSO                    = 9
    integer, parameter, public :: PROCALL_DS9_SLS                      = 10
    integer, parameter, public :: PROCALL_DS9_HEAT                     = 11
    integer, parameter, public :: PROCALL_DS9_COOL                     = 12
    integer, parameter, public :: PROCALL_DS9_RAINBOW                  = 13
    integer, parameter, public :: PROCALL_DS9_STANDARD                 = 14
    integer, parameter, public :: PROCALL_DS9_STAIRCASE                = 15
    integer, parameter, public :: PROCALL_DS9_COLOR                    = 16

    ! IDL-compatible colour patterns.
    integer, parameter, public :: PROCALL_IDL1_B_W_LINEAR              = 17
    integer, parameter, public :: PROCALL_IDL1_BLUE_WHITE              = 18
    integer, parameter, public :: PROCALL_IDL1_GRN_RED_BLU_WHT         = 19
    integer, parameter, public :: PROCALL_IDL1_RED_TEMPERATURE         = 20
    integer, parameter, public :: PROCALL_IDL1_BLUE_GREEN_RED_YELLOW   = 21
    integer, parameter, public :: PROCALL_IDL1_STD_GAMMA_II            = 22
    integer, parameter, public :: PROCALL_IDL1_PRISM                   = 23
    integer, parameter, public :: PROCALL_IDL1_RED_PURPLE              = 24
    integer, parameter, public :: PROCALL_IDL1_GREEN_WHITE_LINEAR      = 25
    integer, parameter, public :: PROCALL_IDL1_RGN_WHT_EXPONENTIAL     = 26
    integer, parameter, public :: PROCALL_IDL1_GREEN_PINK              = 27
    integer, parameter, public :: PROCALL_IDL1_BLUE_RED                = 28
    integer, parameter, public :: PROCALL_IDL1_16_LEVEL                = 29
    integer, parameter, public :: PROCALL_IDL1_RAINBOW                 = 30
    integer, parameter, public :: PROCALL_IDL1_STEPS                   = 31
    integer, parameter, public :: PROCALL_IDL1_STERN_SPECIAL           = 32

    integer, parameter, public :: PROCALL_IDL2_HAZE                    = 33
    integer, parameter, public :: PROCALL_IDL2_BLUE_BASTEL_RED         = 34
    integer, parameter, public :: PROCALL_IDL2_PASTELS                 = 35
    integer, parameter, public :: PROCALL_IDL2_HUE_SAT_LIGHTNESS_1     = 36
    integer, parameter, public :: PROCALL_IDL2_HUE_SAT_LIGHTNESS_2     = 37
    integer, parameter, public :: PROCALL_IDL2_HUE_SAT_VALUE_1         = 38
    integer, parameter, public :: PROCALL_IDL2_HUE_SAT_VALUE_2         = 39
    integer, parameter, public :: PROCALL_IDL2_PURPLE_RED_WITH_STRIPES = 40
    integer, parameter, public :: PROCALL_IDL2_BEACH                   = 41
    integer, parameter, public :: PROCALL_IDL2_MAC_STYLE               = 42
    integer, parameter, public :: PROCALL_IDL2_EOS_A                   = 43
    integer, parameter, public :: PROCALL_IDL2_EOS_B                   = 44
    integer, parameter, public :: PROCALL_IDL2_HARDCANDY               = 45
    integer, parameter, public :: PROCALL_IDL2_NATURE                  = 46
    integer, parameter, public :: PROCALL_IDL2_OCEAN                   = 47
    integer, parameter, public :: PROCALL_IDL2_PEPPERMINT              = 48
    integer, parameter, public :: PROCALL_IDL2_PLASMA                  = 49
    integer, parameter, public :: PROCALL_IDL2_BLUE_RED                = 50
    integer, parameter, public :: PROCALL_IDL2_RAINBOW                 = 51
    integer, parameter, public :: PROCALL_IDL2_BLUE_WAVES              = 52
    integer, parameter, public :: PROCALL_IDL2_VALCANO                 = 53
    integer, parameter, public :: PROCALL_IDL2_WAVES                   = 54
    integer, parameter, public :: PROCALL_IDL2_RAINBOW18               = 55
    integer, parameter, public :: PROCALL_IDL2__RAINBOW                = 56
    integer, parameter, public :: PROCALL_IDL2_ORBIT_VIEWER_COLOR      = 57
    integer, parameter, public :: PROCALL_IDL2_ORBIT_VIEWER_GRAY       = 58

    ! Input keys.
    integer, parameter, public :: PROCALL_KEY_HOME              = int(z'01')
    integer, parameter, public :: PROCALL_KEY_PAGE_UP           = int(z'02')
    integer, parameter, public :: PROCALL_KEY_PAUSE             = int(z'03')
    integer, parameter, public :: PROCALL_KEY_END               = int(z'05')
    integer, parameter, public :: PROCALL_KEY_PAGE_DOWN         = int(z'06')
    integer, parameter, public :: PROCALL_KEY_BACKSPACE         = int(z'08')
    integer, parameter, public :: PROCALL_KEY_TAB               = int(z'09')
    integer, parameter, public :: PROCALL_KEY_ENTER             = int(z'0d')
    integer, parameter, public :: PROCALL_KEY_ESCAPE            = int(z'1b')
    integer, parameter, public :: PROCALL_KEY_RIGHT             = int(z'1c')
    integer, parameter, public :: PROCALL_KEY_LEFT              = int(z'1d')
    integer, parameter, public :: PROCALL_KEY_UP                = int(z'1e')
    integer, parameter, public :: PROCALL_KEY_DOWN              = int(z'1f')
    integer, parameter, public :: PROCALL_KEY_SPACE             = int(z'20')
    integer, parameter, public :: PROCALL_KEY_EXCLAMATION       = int(z'21')
    integer, parameter, public :: PROCALL_KEY_QUOTATION         = int(z'22')
    integer, parameter, public :: PROCALL_KEY_NUMBER            = int(z'23')
    integer, parameter, public :: PROCALL_KEY_DOLLAR            = int(z'24')
    integer, parameter, public :: PROCALL_KEY_PERCENT           = int(z'25')
    integer, parameter, public :: PROCALL_KEY_AMPERSAND         = int(z'26')
    integer, parameter, public :: PROCALL_KEY_SINGLE_QUOTATION  = int(z'27')
    integer, parameter, public :: PROCALL_KEY_PARENTHESES_LEFT  = int(z'28')
    integer, parameter, public :: PROCALL_KEY_PARENTHESES_RIGHT = int(z'29')
    integer, parameter, public :: PROCALL_KEY_ASTERISK          = int(z'2a')
    integer, parameter, public :: PROCALL_KEY_PLUS              = int(z'2b')
    integer, parameter, public :: PROCALL_KEY_COMMA             = int(z'2c')
    integer, parameter, public :: PROCALL_KEY_DASH              = int(z'2d')
    integer, parameter, public :: PROCALL_KEY_PERIOD            = int(z'2e')
    integer, parameter, public :: PROCALL_KEY_SLASH             = int(z'2f')
    integer, parameter, public :: PROCALL_KEY_0                 = int(z'30')
    integer, parameter, public :: PROCALL_KEY_1                 = int(z'31')
    integer, parameter, public :: PROCALL_KEY_2                 = int(z'32')
    integer, parameter, public :: PROCALL_KEY_3                 = int(z'33')
    integer, parameter, public :: PROCALL_KEY_4                 = int(z'34')
    integer, parameter, public :: PROCALL_KEY_5                 = int(z'35')
    integer, parameter, public :: PROCALL_KEY_6                 = int(z'36')
    integer, parameter, public :: PROCALL_KEY_7                 = int(z'37')
    integer, parameter, public :: PROCALL_KEY_8                 = int(z'38')
    integer, parameter, public :: PROCALL_KEY_9                 = int(z'39')
    integer, parameter, public :: PROCALL_KEY_COLON             = int(z'3a')
    integer, parameter, public :: PROCALL_KEY_SEMICOLON         = int(z'3b')
    integer, parameter, public :: PROCALL_KEY_LESS_THAN         = int(z'3c')
    integer, parameter, public :: PROCALL_KEY_EQUALS            = int(z'3d')
    integer, parameter, public :: PROCALL_KEY_GREATER_THAN      = int(z'3e')
    integer, parameter, public :: PROCALL_KEY_QUESTION          = int(z'3f')
    integer, parameter, public :: PROCALL_KEY_AT                = int(z'40')
    integer, parameter, public :: PROCALL_KEY_UPPER_A           = int(z'41')
    integer, parameter, public :: PROCALL_KEY_UPPER_B           = int(z'42')
    integer, parameter, public :: PROCALL_KEY_UPPER_C           = int(z'43')
    integer, parameter, public :: PROCALL_KEY_UPPER_D           = int(z'44')
    integer, parameter, public :: PROCALL_KEY_UPPER_E           = int(z'45')
    integer, parameter, public :: PROCALL_KEY_UPPER_F           = int(z'46')
    integer, parameter, public :: PROCALL_KEY_UPPER_G           = int(z'47')
    integer, parameter, public :: PROCALL_KEY_UPPER_H           = int(z'48')
    integer, parameter, public :: PROCALL_KEY_UPPER_I           = int(z'49')
    integer, parameter, public :: PROCALL_KEY_UPPER_J           = int(z'4a')
    integer, parameter, public :: PROCALL_KEY_UPPER_K           = int(z'4b')
    integer, parameter, public :: PROCALL_KEY_UPPER_L           = int(z'4c')
    integer, parameter, public :: PROCALL_KEY_UPPER_M           = int(z'4d')
    integer, parameter, public :: PROCALL_KEY_UPPER_N           = int(z'4e')
    integer, parameter, public :: PROCALL_KEY_UPPER_O           = int(z'4f')
    integer, parameter, public :: PROCALL_KEY_UPPER_P           = int(z'50')
    integer, parameter, public :: PROCALL_KEY_UPPER_Q           = int(z'51')
    integer, parameter, public :: PROCALL_KEY_UPPER_R           = int(z'52')
    integer, parameter, public :: PROCALL_KEY_UPPER_S           = int(z'53')
    integer, parameter, public :: PROCALL_KEY_UPPER_T           = int(z'54')
    integer, parameter, public :: PROCALL_KEY_UPPER_U           = int(z'55')
    integer, parameter, public :: PROCALL_KEY_UPPER_V           = int(z'56')
    integer, parameter, public :: PROCALL_KEY_UPPER_W           = int(z'57')
    integer, parameter, public :: PROCALL_KEY_UPPER_X           = int(z'58')
    integer, parameter, public :: PROCALL_KEY_UPPER_Y           = int(z'59')
    integer, parameter, public :: PROCALL_KEY_UPPER_Z           = int(z'5a')
    integer, parameter, public :: PROCALL_KEY_BRACKETS_LEFT     = int(z'5b')
    integer, parameter, public :: PROCALL_KEY_BACKSLASH         = int(z'5c')
    integer, parameter, public :: PROCALL_KEY_BRACKETS_RIGHT    = int(z'5d')
    integer, parameter, public :: PROCALL_KEY_CARET             = int(z'5e')
    integer, parameter, public :: PROCALL_KEY_UNDERSCORE        = int(z'5f')
    integer, parameter, public :: PROCALL_KEY_APOSTROPHE        = int(z'60')
    integer, parameter, public :: PROCALL_KEY_LOWER_A           = int(z'61')
    integer, parameter, public :: PROCALL_KEY_LOWER_B           = int(z'62')
    integer, parameter, public :: PROCALL_KEY_LOWER_C           = int(z'63')
    integer, parameter, public :: PROCALL_KEY_LOWER_D           = int(z'64')
    integer, parameter, public :: PROCALL_KEY_LOWER_E           = int(z'65')
    integer, parameter, public :: PROCALL_KEY_LOWER_F           = int(z'66')
    integer, parameter, public :: PROCALL_KEY_LOWER_G           = int(z'67')
    integer, parameter, public :: PROCALL_KEY_LOWER_H           = int(z'68')
    integer, parameter, public :: PROCALL_KEY_LOWER_I           = int(z'69')
    integer, parameter, public :: PROCALL_KEY_LOWER_J           = int(z'6a')
    integer, parameter, public :: PROCALL_KEY_LOWER_K           = int(z'6b')
    integer, parameter, public :: PROCALL_KEY_LOWER_L           = int(z'6c')
    integer, parameter, public :: PROCALL_KEY_LOWER_M           = int(z'6d')
    integer, parameter, public :: PROCALL_KEY_LOWER_N           = int(z'6e')
    integer, parameter, public :: PROCALL_KEY_LOWER_O           = int(z'6f')
    integer, parameter, public :: PROCALL_KEY_LOWER_P           = int(z'70')
    integer, parameter, public :: PROCALL_KEY_LOWER_Q           = int(z'71')
    integer, parameter, public :: PROCALL_KEY_LOWER_R           = int(z'72')
    integer, parameter, public :: PROCALL_KEY_LOWER_S           = int(z'73')
    integer, parameter, public :: PROCALL_KEY_LOWER_T           = int(z'74')
    integer, parameter, public :: PROCALL_KEY_LOWER_U           = int(z'75')
    integer, parameter, public :: PROCALL_KEY_LOWER_V           = int(z'76')
    integer, parameter, public :: PROCALL_KEY_LOWER_W           = int(z'77')
    integer, parameter, public :: PROCALL_KEY_LOWER_X           = int(z'78')
    integer, parameter, public :: PROCALL_KEY_LOWER_Y           = int(z'79')
    integer, parameter, public :: PROCALL_KEY_LOWER_Z           = int(z'7a')
    integer, parameter, public :: PROCALL_KEY_BRACES_LEFT       = int(z'7b')
    integer, parameter, public :: PROCALL_KEY_VERTICAL_BAR      = int(z'7c')
    integer, parameter, public :: PROCALL_KEY_BRACES_RIGHT      = int(z'7d')
    integer, parameter, public :: PROCALL_KEY_TILDE             = int(z'7e')
    integer, parameter, public :: PROCALL_KEY_DELETE            = int(z'7f')

    ! Drawing colours.
    integer, parameter, public :: PROCALL_BLACK    = 0
    integer, parameter, public :: PROCALL_WHITE    = 1
    integer, parameter, public :: PROCALL_RED      = 2
    integer, parameter, public :: PROCALL_GREEN    = 3
    integer, parameter, public :: PROCALL_BLUE     = 4
    integer, parameter, public :: PROCALL_CYAN     = 5
    integer, parameter, public :: PROCALL_MAGENTA  = 6
    integer, parameter, public :: PROCALL_YELLOW   = 7
    integer, parameter, public :: PROCALL_DIM_GRAY = 8
    integer, parameter, public :: PROCALL_GRAY     = 9
    integer, parameter, public :: PROCALL_RED4     = 10
    integer, parameter, public :: PROCALL_GREEN4   = 11
    integer, parameter, public :: PROCALL_BLUE4    = 12
    integer, parameter, public :: PROCALL_CYAN4    = 13
    integer, parameter, public :: PROCALL_MAGENTA4 = 14
    integer, parameter, public :: PROCALL_YELLOW4  = 15

    ! Output image formats.
    character(len=*), parameter, public :: PROCALL_ABEKAS_YUV             = 'ppmtoyuv' // char(0)
    character(len=*), parameter, public :: PROCALL_ATARI_DEGAS            = 'ppmtopi1' // char(0)
    character(len=*), parameter, public :: PROCALL_ATARI_NEOCHROME        = 'ppmtoneo' // char(0)
    character(len=*), parameter, public :: PROCALL_AUTOCAD_DXB            = 'ppmtoacad' // char(0)
    character(len=*), parameter, public :: PROCALL_BERKELEY_YUV           = 'ppmtoeyuv' // char(0)
    character(len=*), parameter, public :: PROCALL_DEC_SIXEL              = 'ppmtosixel' // char(0)
    character(len=*), parameter, public :: PROCALL_GIF                    = 'ppmtogif' // char(0)
    character(len=*), parameter, public :: PROCALL_HP_LASERJET_PCL        = 'ppmtolj' // char(0)
    character(len=*), parameter, public :: PROCALL_HP_PAINTJET            = 'ppmtopj' // char(0)
    character(len=*), parameter, public :: PROCALL_HP_PAINTJET_XL_PCL     = 'ppmtopjxl' // char(0)
    character(len=*), parameter, public :: PROCALL_IFF_ILBM               = 'ppmtoilbm' // char(0)
    character(len=*), parameter, public :: PROCALL_INTERLEAF              = 'ppmtoleaf' // char(0)
    character(len=*), parameter, public :: PROCALL_MACINTOSH_PICT         = 'ppmtopict' // char(0)
    character(len=*), parameter, public :: PROCALL_MAP                    = 'ppmtomap' // char(0)
    character(len=*), parameter, public :: PROCALL_MITSUBISHI_S340        = 'ppmtomitsu' // char(0)
    character(len=*), parameter, public :: PROCALL_MOTIF_UIL_ICON         = 'ppmtouil' // char(0)
    character(len=*), parameter, public :: PROCALL_NCSA_ICR               = 'ppmtoicr' // char(0)
    character(len=*), parameter, public :: PROCALL_PORTABLE_GRAYMAP       = 'ppmtopgm' // char(0)
    character(len=*), parameter, public :: PROCALL_PPC_PAINTBRUSH         = 'ppmtopcx' // char(0)
    character(len=*), parameter, public :: PROCALL_THREE_PORTABLE_GRAYMAP = 'ppmtorgb3' // char(0)
    character(len=*), parameter, public :: PROCALL_TRUEVISION_TARGA       = 'ppmtotga' // char(0)
    character(len=*), parameter, public :: PROCALL_WINDOWS_BITMAP         = 'ppmtobmp' // char(0)
    character(len=*), parameter, public :: PROCALL_WINDOWS_ICON           = 'ppmtowinicon' // char(0)
    character(len=*), parameter, public :: PROCALL_X11_PUZZLE             = 'ppmtopuzz' // char(0)
    character(len=*), parameter, public :: PROCALL_XPM                    = 'ppmtoxpm' // char(0)
    character(len=*), parameter, public :: PROCALL_YUV_TRIPLETS           = 'ppmtoyuvsplit' // char(0)

    character(len=*), parameter, public :: PROCALL_DDIF                   = 'pnmtoddif' // char(0)
    character(len=*), parameter, public :: PROCALL_FIASCO_COMPRESSED      = 'pnmtofiasco' // char(0)
    character(len=*), parameter, public :: PROCALL_FITS                   = 'pnmtofits' // char(0)
    character(len=*), parameter, public :: PROCALL_JBIG                   = 'pnmtojbig' // char(0)
    character(len=*), parameter, public :: PROCALL_JPEG                   = 'pnmtojpeg' // char(0)
    character(len=*), parameter, public :: PROCALL_PALM_PIXMAP            = 'pnmtopalm' // char(0)
    character(len=*), parameter, public :: PROCALL_PLAIN_PNM              = 'pnmtoplainpnm' // char(0)
    character(len=*), parameter, public :: PROCALL_PNG                    = 'pnmtopng' // char(0)
    character(len=*), parameter, public :: PROCALL_POSTSCRIPT             = 'pnmtops' // char(0)
    character(len=*), parameter, public :: PROCALL_RLE                    = 'pnmtorle' // char(0)
    character(len=*), parameter, public :: PROCALL_SGI                    = 'pnmtosgi' // char(0)
    character(len=*), parameter, public :: PROCALL_SOLITAIRE              = 'pnmtosir' // char(0)
    character(len=*), parameter, public :: PROCALL_SUN_RASTER             = 'pnmtorast' // char(0)
    character(len=*), parameter, public :: PROCALL_TIFF                   = 'pnmtotiff' // char(0)
    character(len=*), parameter, public :: PROCALL_TIFF_CMYK              = 'pnmtotiffcmyk' // char(0)
    character(len=*), parameter, public :: PROCALL_X11_WINDOW_DUMP        = 'pnmtoxwd' // char(0)

    public :: arc
    public :: arohd
    public :: circ1
    public :: clsc
    public :: clsx
    public :: copylayer
    public :: drawarc
    public :: drawarrow
    public :: drawcirc
    public :: drawline
    public :: drawlines
    public :: drawnum
    public :: drawpoly
    public :: drawpts
    public :: drawrect
    public :: drawstr
    public :: drawsym
    public :: drawsyms
    public :: fillarc
    public :: fillcirc
    public :: fillpoly
    public :: fillrect
    public :: gclose
    public :: gcloseall
    public :: gclr
    public :: ggetch
    public :: ggetdisplayinfo
    public :: ggetevent
    public :: ggetxpress
    public :: gopen
    public :: gsetbgcolor
    public :: gsetnonblock
    public :: isnan
    public :: layer
    public :: line
    public :: lineto
    public :: makecolor
    public :: moveto
    public :: msleep
    public :: newcolor
    public :: newcoordinate
    public :: newfontset
    public :: newhsvcolor
    public :: newlinestyle
    public :: newpen
    public :: newpencolor
    public :: newrgbcolor
    public :: newwindow
    public :: number
    public :: plot
    public :: plots
    public :: pset
    public :: putimg24
    public :: rtoc
    public :: saveimg
    public :: selwin
    public :: setal
    public :: symbol
    public :: tclr
    public :: vport
    public :: window

    interface
        subroutine arc(xcen, ycen, xrad, yrad, sang, eang, idir)
            real,    intent(in) :: xcen
            real,    intent(in) :: ycen
            real,    intent(in) :: xrad
            real,    intent(in) :: yrad
            real,    intent(in) :: sang
            real,    intent(in) :: eang
            integer, intent(in) :: idir
        end subroutine arc

        subroutine arohd(xs, ys, xt, yt, s, w, n)
            real,    intent(in) :: xs
            real,    intent(in) :: ys
            real,    intent(in) :: xt
            real,    intent(in) :: yt
            real,    intent(in) :: s
            real,    intent(in) :: w
            integer, intent(in) :: n
        end subroutine arohd

        subroutine circ1(xc, yc, r)
            real, intent(in) :: xc
            real, intent(in) :: yc
            real, intent(in) :: r
        end subroutine circ1

        subroutine clsc()
        end subroutine clsc

        subroutine clsx()
        end subroutine clsx

        subroutine copylayer(nwin, lysrc, lydest)
            integer, intent(in) :: nwin
            integer, intent(in) :: lysrc
            integer, intent(in) :: lydest
        end subroutine copylayer

        subroutine drawarc(nwin, xcen, ycen, xrad, yrad, sang, eang, idir)
            integer, intent(in) :: nwin
            real,    intent(in) :: xcen
            real,    intent(in) :: ycen
            real,    intent(in) :: xrad
            real,    intent(in) :: yrad
            real,    intent(in) :: sang
            real,    intent(in) :: eang
            integer, intent(in) :: idir
        end subroutine drawarc

        subroutine drawarrow(nwin, xs, ys, xt, yt, s, w, n)
            integer, intent(in) :: nwin
            real,    intent(in) :: xs
            real,    intent(in) :: ys
            real,    intent(in) :: xt
            real,    intent(in) :: yt
            real,    intent(in) :: s
            real,    intent(in) :: w
            integer, intent(in) :: n
        end subroutine drawarrow

        subroutine drawcirc(nwin, xcen, ycen, xrad, yrad)
            integer, intent(in) :: nwin
            real,    intent(in) :: xcen
            real,    intent(in) :: ycen
            real,    intent(in) :: xrad
            real,    intent(in) :: yrad
        end subroutine drawcirc

        subroutine drawline(nwin, xg0, yg0, xg1, yg1)
            integer, intent(in) :: nwin
            real,    intent(in) :: xg0
            real,    intent(in) :: yg0
            real,    intent(in) :: xg1
            real,    intent(in) :: yg1
        end subroutine drawline

        subroutine drawlines(nwin, x, y, n)
            integer, intent(in)   :: nwin
            real,    intent(inout) :: x(*)
            real,    intent(inout) :: y(*)
            integer, intent(in)    :: n
        end subroutine drawlines

        subroutine drawnum(nwin, xg, yg, size, v, theta, len)
            integer, intent(in) :: nwin
            real,    intent(in) :: xg
            real,    intent(in) :: yg
            real,    intent(in) :: size
            real,    intent(in) :: v
            real,    intent(in) :: theta
            integer, intent(in) :: len
        end subroutine drawnum

        subroutine drawpoly(nwin, x, y, n)
            integer, intent(in)    :: nwin
            real,    intent(inout) :: x(*)
            real,    intent(inout) :: y(*)
            integer, intent(in)    :: n
        end subroutine drawpoly

        subroutine drawpts(nwin, x, y, n)
            integer, intent(in)    :: nwin
            real,    intent(inout) :: x(*)
            real,    intent(inout) :: y(*)
            integer, intent(in)    :: n
        end subroutine drawpts

        subroutine drawrect(nwin, x, y, w, h)
            integer, intent(in) :: nwin
            real,    intent(in) :: x
            real,    intent(in) :: y
            real,    intent(in) :: w
            real,    intent(in) :: h
        end subroutine drawrect

        subroutine drawstr(nwin, xg, yg, size, str, theta, len)
            integer,          intent(in) :: nwin
            real,             intent(in) :: xg
            real,             intent(in) :: yg
            real,             intent(in) :: size
            character(len=*), intent(in) :: str
            real,             intent(in) :: theta
            integer,          intent(in) :: len
        end subroutine drawstr

        subroutine drawsym(nwin, xg, yg, size, nsym)
            integer, intent(in) :: nwin
            real,    intent(in) :: xg
            real,    intent(in) :: yg
            real,    intent(in) :: size
            integer, intent(in) :: nsym
        end subroutine drawsym

        subroutine drawsyms(nwin, x, y, n, size, nsym)
            integer, intent(in)    :: nwin
            real,    intent(inout) :: x(*)
            real,    intent(inout) :: y(*)
            integer, intent(in)    :: n
            real,    intent(in)    :: size
            integer, intent(in)    :: nsym
        end subroutine drawsyms

        subroutine fillarc(nwin, xcen, ycen, xrad, yrad, sang, eang, idir)
            integer, intent(in) :: nwin
            real,    intent(in) :: xcen
            real,    intent(in) :: ycen
            real,    intent(in) :: xrad
            real,    intent(in) :: yrad
            real,    intent(in) :: sang
            real,    intent(in) :: eang
            integer, intent(in) :: idir
        end subroutine fillarc

        subroutine fillcirc(nwin, xcen, ycen, xrad, yrad)
            integer, intent(in) :: nwin
            real,    intent(in) :: xcen
            real,    intent(in) :: ycen
            real,    intent(in) :: xrad
            real,    intent(in) :: yrad
        end subroutine fillcirc

        subroutine fillpoly(nwin, x, y, n, i)
            integer, intent(in)    :: nwin
            real,    intent(inout) :: x(*)
            real,    intent(inout) :: y(*)
            integer, intent(in)    :: n
            integer, intent(in)    :: i
        end subroutine fillpoly

        subroutine fillrect(nwin, x, y, w, h)
            integer, intent(in) :: nwin
            real,    intent(in) :: x
            real,    intent(in) :: y
            real,    intent(in) :: w
            real,    intent(in) :: h
        end subroutine fillrect

        subroutine gclose(nwin)
            integer, intent(in) :: nwin
        end subroutine gclose

        subroutine gcloseall()
        end subroutine gcloseall

        subroutine gclr(nwin)
            integer, intent(in) :: nwin
        end subroutine gclr

        subroutine ggetch(key)
            integer, intent(out) :: key
        end subroutine ggetch

        subroutine ggetdisplayinfo(ndepth, nrwidth, nrheight)
            integer, intent(out) :: ndepth
            integer, intent(out) :: nrwidth
            integer, intent(out) :: nrheight
        end subroutine ggetdisplayinfo

        subroutine ggetevent(nwin, ntype, nbutton, xg, yg)
            integer, intent(in)  :: nwin
            integer, intent(out) :: ntype
            integer, intent(out) :: nbutton
            real,    intent(out) :: xg
            real,    intent(out) :: yg
        end subroutine ggetevent

        subroutine ggetxpress(nwin, ntype, nbutton, xg, yg)
            integer, intent(in)  :: nwin
            integer, intent(out) :: ntype
            integer, intent(out) :: nbutton
            real,    intent(out) :: xg
            real,    intent(out) :: yg
        end subroutine ggetxpress

        subroutine gopen(nxsize, nysize, nwin)
            integer, intent(in)  :: nxsize
            integer, intent(in)  :: nysize
            integer, intent(out) :: nwin
        end subroutine gopen

        subroutine gsetbgcolor(nwin, strc)
            integer,          intent(in) :: nwin
            character(len=*), intent(in) :: strc
        end subroutine gsetbgcolor

        subroutine gsetnonblock(iflag)
            integer, intent(in) :: iflag
        end subroutine gsetnonblock

        subroutine isnan(v, iflag)
            real,    intent(in)  :: v
            integer, intent(out) :: iflag
        end subroutine isnan

        subroutine layer(nwin, lys, lyw)
            integer, intent(in) :: nwin
            integer, intent(in) :: lys
            integer, intent(in) :: lyw
        end subroutine layer

        subroutine line(nwin, xg, yg, mode)
            integer, intent(in) :: nwin
            real,    intent(in) :: xg
            real,    intent(in) :: yg
            integer, intent(in) :: mode
        end subroutine line

        subroutine lineto(nwin, xg, yg)
            integer, intent(in) :: nwin
            real,    intent(in) :: xg
            real,    intent(in) :: yg
        end subroutine lineto

        subroutine makecolor(ncolormode, dmin, dmax, data, nr, ng, nb)
            integer, intent(in)  :: ncolormode
            real,    intent(in)  :: dmin
            real,    intent(in)  :: dmax
            real,    intent(in)  :: data
            integer, intent(out) :: nr
            integer, intent(out) :: ng
            integer, intent(out) :: nb
        end subroutine makecolor

        subroutine moveto(nwin, xg, yg)
            integer, intent(in) :: nwin
            real,    intent(in) :: xg
            real,    intent(in) :: yg
        end subroutine moveto

        subroutine msleep(ms)
            integer, intent(in) :: ms
        end subroutine msleep

        subroutine newcolor(nwin, strc)
            integer,          intent(in) :: nwin
            character(len=*), intent(in) :: strc
        end subroutine newcolor

        subroutine newcoordinate(nwin, nxw, nyw, xa, ya, xscale, yscale)
            integer, intent(in) :: nwin
            integer, intent(in) :: nxw
            integer, intent(in) :: nyw
            real,    intent(in) :: xa
            real,    intent(in) :: ya
            real,    intent(in) :: xscale
            real,    intent(in) :: yscale
        end subroutine newcoordinate

        subroutine newfontset(nwin, fontset, nstatus)
            integer,          intent(in)  :: nwin
            character(len=*), intent(in)  :: fontset
            integer,          intent(out) :: nstatus
        end subroutine newfontset

        subroutine newhsvcolor(nwin, nh, ns, nv)
            integer, intent(in) :: nwin
            integer, intent(in) :: nh
            integer, intent(in) :: ns
            integer, intent(in) :: nv
        end subroutine newhsvcolor

        subroutine newlinestyle(nwin, nstyle)
            integer, intent(in) :: nwin
            integer, intent(in) :: nstyle
        end subroutine newlinestyle

        subroutine newpen(nc)
            integer, intent(in) :: nc
        end subroutine newpen

        subroutine newpencolor(nwin, nc)
            integer, intent(in) :: nwin
            integer, intent(in) :: nc
        end subroutine newpencolor

        subroutine newrgbcolor(nwin, nr, ng, nb)
            integer, intent(in) :: nwin
            integer, intent(in) :: nr
            integer, intent(in) :: ng
            integer, intent(in) :: nb
        end subroutine newrgbcolor

        subroutine newwindow(nwin, xs, ys, xe, ye)
            integer, intent(in) :: nwin
            real,    intent(in) :: xs
            real,    intent(in) :: ys
            real,    intent(in) :: xe
            real,    intent(in) :: ye
        end subroutine newwindow

        subroutine number(xg, yg, size, v, theta, len)
            real,    intent(in) :: xg
            real,    intent(in) :: yg
            real,    intent(in) :: size
            real,    intent(in) :: v
            real,    intent(in) :: theta
            integer, intent(in) :: len
        end subroutine number

        subroutine plot(xg, yg, mode)
            real,    intent(in) :: xg
            real,    intent(in) :: yg
            integer, intent(in) :: mode
        end subroutine plot

        subroutine plots()
        end subroutine plots

        subroutine pset(nwin, xg, yg)
            integer, intent(in) :: nwin
            real,    intent(in) :: xg
            real,    intent(in) :: yg
        end subroutine pset

        subroutine putimg24(nwin, x, y, nw, nh, nbuf)
            integer, intent(in)    :: nwin
            real,    intent(in)    :: x
            real,    intent(in)    :: y
            integer, intent(in)    :: nw
            integer, intent(in)    :: nh
            integer, intent(inout) :: nbuf(*)
        end subroutine putimg24

        subroutine rtoc(v, n, ns, str, m)
            real,             intent(in)    :: v
            integer,          intent(in)    :: n
            integer,          intent(in)    :: ns
            character(len=*), intent(inout) :: str
            integer,          intent(out)   :: m
        end subroutine rtoc

        subroutine saveimg(nwin, ly, xs, ys, xe, ye, fname, n, conv, nd)
            integer,          intent(in) :: nwin
            integer,          intent(in) :: ly
            real,             intent(in) :: xs
            real,             intent(in) :: ys
            real,             intent(in) :: xe
            real,             intent(in) :: ye
            character(len=*), intent(in) :: fname
            integer,          intent(in) :: n
            character(len=*), intent(in) :: conv
            integer,          intent(in) :: nd
        end subroutine saveimg

        subroutine selwin(nwin)
            integer, intent(in) :: nwin
        end subroutine selwin

        subroutine setal()
        end subroutine setal

        subroutine symbol(xg, yg, size, nstr, theta, len)
            real,             intent(in) :: xg
            real,             intent(in) :: yg
            real,             intent(in) :: size
            character(len=*), intent(in) :: nstr
            real,             intent(in) :: theta
            integer,          intent(in) :: len
        end subroutine symbol

        subroutine tclr()
        end subroutine tclr

        subroutine vport()
        end subroutine vport

        subroutine window(xs, ys, xe, ye)
            real, intent(in) :: xs
            real, intent(in) :: ys
            real, intent(in) :: xe
            real, intent(in) :: ye
        end subroutine window
    end interface
end module procall
