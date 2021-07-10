! munch.f90
!
! Munching squares, renders a graphical pattern where each pixel is colored by
! the value of x xor y from a colour palette.
!
! Author:  Philipp Engel
! Licence: ISC
program main
    use, intrinsic :: iso_c_binding, only: c_null_char
    use :: procall
    implicit none

    character(len=*), parameter :: WIN_TITLE = 'Fortran Munching Squares'
    integer,          parameter :: WIN_SIZE  = 500

    integer :: key
    integer :: r, g, b
    integer :: win
    integer :: x, y

    call gopen(WIN_SIZE, WIN_SIZE, win)             ! Open X11 window.
    call winname(win, WIN_TITLE // c_null_char)     ! Set window title.
    call gsetbgcolor(win, 'black' // c_null_char)   ! Set background colour.
    call gclr(win)                                  ! Clear drawing area.

    do y = 0, WIN_SIZE - 1
        do x = 0, WIN_SIZE - 1
            call makecolor(PROCALL_IDL2_BLUE_RED, real(WIN_SIZE), 0.0, real(ieor(x, y)), r, g, b)
            call newrgbcolor(win, r, g, b)
            call pset(win, real(x), real(y))
        end do
    end do

    call ggetch(key)
    call gcloseall()
end program main
