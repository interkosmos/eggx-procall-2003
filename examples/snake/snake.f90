! snake.f90
!
! Simple snake game in Fortran, using ProCALL.
!
! Author:  Philipp Engel
! Licence: ISC
module snake
    implicit none
    private

    integer, parameter, public :: DIR_NONE  = 0
    integer, parameter, public :: DIR_UP    = 1
    integer, parameter, public :: DIR_DOWN  = 2
    integer, parameter, public :: DIR_LEFT  = 3
    integer, parameter, public :: DIR_RIGHT = 4

    type, public :: part_type
        integer                  :: x
        integer                  :: y
        type(part_type), pointer :: prev
        type(part_type), pointer :: next
    end type part_type

    type, public :: body_type
        integer                  :: length
        type(part_type), pointer :: head
        type(part_type), pointer :: tail
    end type body_type

    type, public :: food_type
        integer :: x
        integer :: y
    end type food_type

    type, public :: world_type
        integer         :: width
        integer         :: height
        type(food_type) :: food
    end type world_type

    public :: snake_add
    public :: snake_delete
    public :: snake_move
    public :: snake_new
    public :: snake_new_food
contains
    function snake_move(body, world, dir)
        type(body_type),  intent(inout) :: body
        type(world_type), intent(inout) :: world
        integer,          intent(in)    :: dir
        logical                         :: snake_move
        integer                         :: x, y
        type(part_type), pointer        :: ptr

        snake_move = .false.
        if (body%length == 0) return

        x = body%head%x
        y = body%head%y

        select case (dir)
            case (DIR_UP)
                y = y - 1
            case (DIR_DOWN)
                y = y + 1
            case (DIR_LEFT)
                x = x - 1
            case (DIR_RIGHT)
                x = x + 1
            case default
                return
        end select

        ! Out of bounds.
        if (x < 0 .or. x >= world%width .or. &
            y < 0 .or. y >= world%height) return

        ! Check for collision with snake body.
        ptr => body%head

        do while (associated(ptr))
            if (x == ptr%x .and. y == ptr%y) return
            ptr => ptr%next
        end do

        ! Eat if food hit.
        if (x == world%food%x .and. y == world%food%y) then
            call snake_eat(body)
            call snake_new_food(body, world)
        end if

        ! Move whole snake body.
        ptr => body%tail

        do while (associated(ptr%prev))
            ptr%x = ptr%prev%x
            ptr%y = ptr%prev%y
            ptr => ptr%prev
        end do

        ! Set head to new position.
        body%head%x = x
        body%head%y = y

        snake_move = .true.
    end function snake_move

    subroutine snake_add(body, part)
        type(body_type), intent(inout) :: body
        type(part_type), intent(inout) :: part

        if (.not. associated(body%head)) then
            allocate (body%head, source=part)
            body%tail => body%head
            body%length = 1
            return
        end if

        if (associated(body%tail%next)) return
        allocate (body%tail%next, source=part)
        body%tail => body%tail%next
        body%length = body%length + 1
    end subroutine snake_add

    subroutine snake_delete(body)
        type(body_type), intent(inout) :: body
        type(part_type), pointer       :: ptr, tmp

        ptr => body%tail

        do while (associated(ptr))
            tmp => ptr%prev
            deallocate (ptr)
            ptr => tmp
        end do
    end subroutine snake_delete

    subroutine snake_eat(body)
        type(body_type),  intent(inout) :: body
        type(part_type)                 :: part

        part = part_type(body%tail%x, body%tail%y, body%tail, null())
        call snake_add(body, part)
    end subroutine snake_eat

    subroutine snake_new(body)
        type(body_type), intent(inout) :: body

        body%length = 0
        body%head => null()
        body%tail => null()
    end subroutine snake_new

    subroutine snake_new_food(body, world)
        type(body_type),  intent(inout) :: body
        type(world_type), intent(inout) :: world
        integer                         :: x, y
        real                            :: r(2)
        type(part_type), pointer        :: ptr

        loop: do
            call random_number(r)
            x = int(r(1) * world%width)
            y = int(r(2) * world%height)

            ptr => body%head

            do while (associated(ptr))
                if (ptr%x == x .and. ptr%y == y) cycle loop
                ptr => ptr%next
            end do

            exit
        end do loop

        world%food%x = x
        world%food%y = y
    end subroutine snake_new_food
end module snake

program main
    use, intrinsic :: iso_c_binding, only: c_null_char
    use :: procall
    use :: snake
    implicit none

    character(len=*), parameter :: FONT_NAME = '-*-helvetica-medium-o-*-*-24-*-*-*-*-*-*-*'

    integer, parameter :: WORLD_WIDTH  = 64
    integer, parameter :: WORLD_HEIGHT = 48
    integer, parameter :: BLOCK_SIZE   = 10

    integer, parameter :: WIN_WIDTH  = WORLD_WIDTH * BLOCK_SIZE
    integer, parameter :: WIN_HEIGHT = WORLD_HEIGHT * BLOCK_SIZE
    integer, parameter :: DELAY      = 50

    integer          :: dir, next
    integer          :: dt
    integer          :: key
    integer          :: win
    real             :: t1, t2
    type(body_type)  :: body
    type(part_type)  :: head
    type(world_type) :: world

    call random_seed()

    ! Open X11 window.
    if (open_window(win, WIN_WIDTH, WIN_HEIGHT, 'Fortran Snake', FONT_NAME) /= 0) then
        print '(3a)', 'Error: Loading font "', trim(FONT_NAME), '" failed'
        call gcloseall()
        stop
    end if

    ! Initialise snake.
    call snake_new(body)
    head = part_type(WORLD_WIDTH / 2, WORLD_HEIGHT / 2, null(), null())
    call snake_add(body, head)

    ! Initialise world.
    world = world_type(width  = WORLD_WIDTH, &
                       height = WORLD_HEIGHT, &
                       food   = food_type(0, 0))
    call snake_new_food(body, world)

    ! Draw world to screen and get initial direction.
    call draw(win, world, body)

    do
        call ggetch(key)
        dir = direction(key)
        if (dir /= DIR_NONE) exit
    end do

    ! Enable non-blocking keyboard input.
    call gsetnonblock(1)

    do
        call cpu_time(t1)

        call ggetch(key)
        if (key == PROCALL_KEY_ESCAPE) exit
        next = direction(key)
        if (next /= DIR_NONE) dir = next

        if (.not. snake_move(body, world, dir)) then
            call game_over(win)
            exit
        end if
        call draw(win, world, body)

        call cpu_time(t2)
        dt = int((t2 - t1) * 1000)
        if (dt < DELAY) call msleep(DELAY - dt)
    end do

    call snake_delete(body)
    call gcloseall()
contains
    function direction(key)
        integer, intent(in) :: key
        integer             :: direction

        direction = DIR_NONE

        select case (key)
            case (PROCALL_KEY_RIGHT)
                direction  = DIR_RIGHT

            case (PROCALL_KEY_LEFT)
                direction  = DIR_LEFT

            case (PROCALL_KEY_UP)
                direction = DIR_DOWN

            case (PROCALL_KEY_DOWN)
                direction = DIR_UP
        end select
    end function direction

    function open_window(win, width, height, title, font) result(rc)
        integer,          intent(inout) :: win
        integer,          intent(in)    :: width
        integer,          intent(in)    :: height
        character(len=*), intent(in)    :: title
        character(len=*), intent(in)    :: font
        integer                         :: rc

        rc = -1
        call gopen(width, height, win)
        call winname(win, title // c_null_char)
        call gsetbgcolor(win, 'black' // c_null_char)
        call layer(win, 0, 1)

        call newfontset(win, font // c_null_char, rc)
        if (rc /= 0) return
        rc = 0
    end function open_window

    subroutine draw(win, world, body)
        integer,          intent(inout) :: win
        type(world_type), intent(inout) :: world
        type(body_type),  intent(inout) :: body
        type(part_type), pointer        :: ptr

        call gclr(win)

        ! Draw snake.
        call newpencolor(win, PROCALL_GREEN)
        ptr => body%head

        do while (associated(ptr))
            call fillrect(win, &
                          real(ptr%x * BLOCK_SIZE - 1), &
                          real(ptr%y * BLOCK_SIZE - 1), &
                          real(BLOCK_SIZE), &
                          real(BLOCK_SIZE))
            ptr => ptr%next
        end do

        ! Draw food.
        call newpencolor(win, PROCALL_RED)
        call fillrect(win, &
                      real(world%food%x * BLOCK_SIZE - 1), &
                      real(world%food%y * BLOCK_SIZE - 1), &
                      real(BLOCK_SIZE), &
                      real(BLOCK_SIZE))

        call copylayer(win, 1, 0)
    end subroutine draw

    subroutine game_over(win)
        integer, intent(inout) :: win
        integer                :: key

        call gclr(win)
        call newpencolor(win, PROCALL_WHITE)
        call drawstr(nwin  = win, &
                     xg    = 250.0, &
                     yg    = 240.0, &
                     size  = 0.0, &
                     str   = 'Game Over!' // c_null_char, &
                     theta = 0.0, &
                     len   = -1)
        call copylayer(win, 1, 0)

        call gsetnonblock(0)

        do
            call ggetch(key)
            if (key == PROCALL_KEY_ESCAPE) exit
        end do
    end subroutine game_over
end program main
