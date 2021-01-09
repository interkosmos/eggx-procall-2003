! starfield.f90
!
! X11 starfield demo for EGGX/ProCALL.
!
! Author:  Philipp Engel
! Licence: ISC
module starfield
    implicit none
    private

    type, public :: point_type
        real :: x, y
    end type point_type

    type, public :: star_type
        real :: x, y, z
    end type star_type

    public :: starfield_init
    public :: starfield_move
    public :: starfield_project
contains
    subroutine starfield_init(stars, width, height, max_depth)
        !! Initialises the starfield to given viewport width and height, with
        !! respect to maximum depth in Z.
        type(star_type), intent(inout) :: stars(:)
        real,            intent(in)    :: width
        real,            intent(in)    :: height
        real,            intent(in)    :: max_depth
        integer                        :: i
        real                           :: r(3)

        do i = 1, size(stars)
            call random_number(r)
            stars(i) = star_type(x = (width / 2) - (r(1) * width), &
                                 y = (height / 2) - (r(2) * height), &
                                 z = r(3) * max_depth)
        end do
    end subroutine starfield_init

    subroutine starfield_move(stars, width, height, max_depth, dz)
        !! Moves starfield in camera direction by `dz`. Stars passing the camera
        !! will be reset to a random X, Y position, and Z set to `max_depth`.
        type(star_type), intent(inout) :: stars(:)
        real,            intent(in)    :: width
        real,            intent(in)    :: height
        real,            intent(in)    :: max_depth
        real,            intent(in)    :: dz
        integer                        :: i
        real                           :: r(2)

        do i = 1, size(stars)
            stars(i)%z = stars(i)%z - dz

            if (stars(i)%z < 0.0) then
                call random_number(r)
                stars(i) = star_type(x = (width / 2) - (r(1) * width), &
                                     y = (height / 2) - (r(2) * height), &
                                     z = max_depth)
            end if
        end do
    end subroutine starfield_move

    subroutine starfield_project(stars, width, height, points)
        !! Projects star coordinates from 3D into 2D.
        type(star_type),  intent(inout) :: stars(:)
        real,             intent(in)    :: width
        real,             intent(in)    :: height
        type(point_type), intent(inout) :: points(:)
        integer                         :: i
        real                            :: k

        do i = 1, size(stars)
            k = 128.0 / stars(i)%z
            points(i)%x = stars(i)%x * k + (width / 2)
            points(i)%y = stars(i)%y * k + (height / 2)
        end do
    end subroutine starfield_project
end module starfield

program main
    use, intrinsic :: iso_c_binding, only: c_null_char
    use :: procall
    use :: starfield
    implicit none

    real,    parameter :: WIDTH      = 640.                 ! Viewport width.
    real,    parameter :: HEIGHT     = 480.                 ! Viewport height.
    real,    parameter :: MAX_DEPTH  = 800.                 ! Max. Z of stars.
    real,    parameter :: DZ         = 2.0                  ! Stars movement in Z.
    integer, parameter :: NSTARS     = 128                  ! Number of stars.
    integer, parameter :: WIN_WIDTH  = int(WIDTH)           ! Window width.
    integer, parameter :: WIN_HEIGHT = int(HEIGHT)          ! Window height.
    integer, parameter :: DELAY      = 10                   ! Frame delay in msec.

    type(star_type)  :: stars(NSTARS)
    type(point_type) :: points(NSTARS)
    integer          :: i
    integer          :: r, g, b
    integer          :: win

    ! Initialise PRNG.
    call random_seed()

    ! Initialise the starfield.
    call starfield_init(stars, WIDTH, HEIGHT, MAX_DEPTH)

    call gopen(WIN_WIDTH, WIN_HEIGHT, win)                  ! Open X11 window.
    call winname(win, 'Fortran Starfield' // c_null_char)   ! Set window title.
    call gsetbgcolor(win, 'black' // c_null_char)           ! Set background colour.
    call layer(win, 0, 1)                                   ! Enable double buffering.

    do
        ! Move starfield and project from 3D to 2D.
        call starfield_move(stars, WIDTH, HEIGHT, MAX_DEPTH, DZ)
        call starfield_project(stars, WIDTH, HEIGHT, points)

        ! Clear draw area and draw points.
        call gclr(win)

        do i = 1, NSTARS
            ! Get RGB values from colour gradient, set pen colour, and draw
            ! single pixel. Stars farther away will be darker.
            call makecolor(0, MAX_DEPTH * 1.5, 0.0, stars(i)%z, r, g, b)
            call newrgbcolor(win, r, g, b)
            call pset(win, points(i)%x - 1, points(i)%y - 1)
        end do

        ! Copy layer to window and sleep.
        call copylayer(win, 1, 0)
        call msleep(DELAY)
    end do

    ! Close all windows.
    call gcloseall()
end program main
