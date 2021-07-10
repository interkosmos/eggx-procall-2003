! tiles.f90
!
! EGGX/ProCALL example that demonstrates tile-based blitting and font rendering.
! Requires the X11 font "New Century Schoolbook".
!
! Images are loaded from file `tiles.bin`. The file contains 10 tiles in 32x32
! size, stored as a continuous byte stream (4 bytes per pixel: A, R, G, B).
!
! The images are taken from:
!
!   http://www3.wind.ne.jp/DENZI/diary/
!
! Author:  Philipp Engel
! Licence: ISC
program main
    use, intrinsic :: iso_c_binding, only: c_char, c_null_char
    use, intrinsic :: iso_fortran_env, only: stderr => error_unit
    use :: eggx
    use :: procall
    implicit none

    character(len=*), parameter :: FONT_NAME = '-*-new century schoolbook-medium-r-*-*-18-*-*-*-*-*-*-*'
    character(len=*), parameter :: FILE_NAME = 'tiles.bin'
    character(len=*), parameter :: MESSAGE   = 'Hello, from Fortran and EGGX/ProCALL!'
    character(len=*), parameter :: WIN_TITLE = 'Fortran EGGX/ProCALL'

    integer, parameter :: WIN_WIDTH   = 640
    integer, parameter :: WIN_HEIGHT  = 480
    integer, parameter :: NTILES      = 10
    integer, parameter :: TILE_WIDTH  = 32
    integer, parameter :: TILE_HEIGHT = 32
    integer, parameter :: TILE_SIZE   = 4 * TILE_WIDTH * TILE_HEIGHT
    integer, parameter :: NCOLUMNS    = WIN_WIDTH / TILE_WIDTH
    integer, parameter :: NROWS       = WIN_HEIGHT / TILE_HEIGHT

    ! Tile pixels (ARGB).
    type :: tile_type
        character(len=1) :: data(TILE_SIZE)
    end type tile_type

    integer         :: key
    integer         :: n
    integer         :: rc
    integer         :: x, y
    integer         :: win
    type(tile_type) :: tiles(NTILES)

    ! Load tiles from file.
    rc = load_tiles(FILE_NAME, tiles, TILE_SIZE)

    if (rc /= 0) then
        write (stderr, '(3a)') 'Error: Loading file "', trim(FILE_NAME), '" failed.'
        stop
    end if

    ! Initialise EGGX/ProCALL.
    call gopen(WIN_WIDTH, WIN_HEIGHT, win)              ! Open X11 window.
    call winname(win, WIN_TITLE // c_null_char)         ! Set window title.
    call newfontset(win, FONT_NAME // c_null_char, rc)  ! Load font.

    if (rc /= 0) then
        write (stderr, '(3a)') 'Error: Font "', trim(FONT_NAME), '" not found.'
        stop
    end if

    call gsetbgcolor(win, 'black' // c_null_char)       ! Set background colour.
    call newpencolor(win, 0)                            ! Set foreground colour.
    call layer(win, 0, 1)                               ! Enable double buffering.
    call gclr(win)                                      ! Clear screen.

    ! Draw tiles (screen size: 20 x 15 tiles).
    do y = 0, NROWS - 1
        do x = 0, NCOLUMNS - 1
            ! Default tile.
            n = 1

            ! Fill center.
            if (x >= 4 .and. x <= 15 .and. y >= 4 .and. y <= 10) n = 2

            ! Top border.
            if (x == 3 .and. y == 11) n = 3
            if (x >= 4 .and. x <= 15 .and. y == 11) n = 4
            if (x == 16 .and. y == 11) n = 5

            ! Left/right border.
            if (x == 3 .and. y >= 4 .and. y <= 10) n = 6
            if (x == 16 .and. y >= 4 .and. y <= 10) n = 7

            ! Bottom border.
            if (x == 3 .and. y == 3) n = 8
            if (x >= 4 .and. x <= 15 .and. y == 3) n = 9
            if (x == 16 .and. y == 3) n = 10

            ! Copy tile `n` to layer.
            rc = eggx_gputimage(win, &
                                dble(x * TILE_WIDTH), dble(y * TILE_HEIGHT), &
                                tiles(n)%data, &
                                TILE_WIDTH, TILE_HEIGHT, &
                                0)
        end do
    end do

    ! Draw message string.
    call drawstr(nwin  = win, &
                 xg    = real(4 * TILE_WIDTH), &
                 yg    = real(9 * TILE_HEIGHT), &
                 size  = 0.0, &
                 str   = MESSAGE // c_null_char, &
                 theta = 0.0, &
                 len   = -1)

    call copylayer(win, 1, 0)   ! Copy layer to screen.
    call ggetch(key)            ! Wait for input.
    call gcloseall()            ! Quit.
contains
    function load_tiles(file_path, tiles, tile_size)
        !! Loads tile images from binary file.
        character(len=*), intent(in)    :: file_path
        type(tile_type),  intent(inout) :: tiles(:)
        integer,          intent(in)    :: tile_size
        integer                         :: load_tiles
        integer                         :: i, j, fu, rc

        load_tiles = -1

        open (access='stream', action='read', file=file_path, form='unformatted', &
              iostat=rc, newunit=fu, status='old')

        if (rc /= 0) return

        do j = 1, size(tiles)
            do i = 1, tile_size, 4
                ! Read 4 bytes.
                read (fu) tiles(j)%data(i), &     ! A
                          tiles(j)%data(i + 1), & ! R
                          tiles(j)%data(i + 2), & ! G
                          tiles(j)%data(i + 3)    ! B
            end do
        end do

        close (fu)

        load_tiles = 0
    end function load_tiles
end program main
