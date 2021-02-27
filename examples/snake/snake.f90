! snake.f90
!
! Simple snake game in Fortran, using ProCALL.
! Artwork by eugeneloza (public domain):
!
!   https://opengameart.org/content/snake-sprites-sound
!
! Author:  Philipp Engel
! Licence: ISC
module snake
    implicit none
    private

    integer, parameter, public :: DIR_NONE  = -1
    integer, parameter, public :: DIR_UP    = 0
    integer, parameter, public :: DIR_RIGHT = 1
    integer, parameter, public :: DIR_DOWN  = 2
    integer, parameter, public :: DIR_LEFT  = 3

    type, public :: part_type
        integer                  :: x
        integer                  :: y
        integer                  :: dir
        type(part_type), pointer :: prev => null()
        type(part_type), pointer :: next => null()
    end type part_type

    type, public :: body_type
        integer                  :: length
        type(part_type), pointer :: head => null()
        type(part_type), pointer :: tail => null()
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
    function snake_move(body, world)
        type(body_type),  intent(inout) :: body
        type(world_type), intent(inout) :: world
        logical                         :: snake_move
        integer                         :: x, y
        type(part_type), pointer        :: ptr

        snake_move = .false.
        if (body%length == 0) return

        x = body%head%x
        y = body%head%y

        select case (body%head%dir)
            case (DIR_UP)
                y = y + 1
            case (DIR_DOWN)
                y = y - 1
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

        ! Eat if food is hit.
        if (x == world%food%x .and. y == world%food%y) then
            call snake_eat(body)
            call snake_new_food(body, world)
        end if

        ! Move whole snake body.
        ptr => body%tail

        do while (associated(ptr%prev))
            ptr%x   = ptr%prev%x
            ptr%y   = ptr%prev%y
            ptr%dir = ptr%prev%dir
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

        part = part_type(body%tail%x, body%tail%y, DIR_NONE, body%tail, null())
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

module ui
    use, intrinsic :: iso_c_binding, only: c_null_char
    use :: procall
    use :: snake
    implicit none
    private

    integer, parameter, public :: BLOCK_SIZE  = 16
    integer, parameter, public :: SPRITE_SIZE = 3 * BLOCK_SIZE * BLOCK_SIZE

    integer, parameter, public :: SPRITE_HEAD = 1
    integer, parameter, public :: SPRITE_TAIL = 5

    integer, parameter, public :: SPRITE_HEAD_UP     = 1
    integer, parameter, public :: SPRITE_HEAD_RIGHT  = 2
    integer, parameter, public :: SPRITE_HEAD_DOWN   = 3
    integer, parameter, public :: SPRITE_HEAD_LEFT   = 4
    integer, parameter, public :: SPRITE_TAIL_UP     = 5
    integer, parameter, public :: SPRITE_TAIL_RIGHT  = 6
    integer, parameter, public :: SPRITE_TAIL_DOWN   = 7
    integer, parameter, public :: SPRITE_TAIL_LEFT   = 8
    integer, parameter, public :: SPRITE_CURVE_UP    = 9
    integer, parameter, public :: SPRITE_CURVE_RIGHT = 10
    integer, parameter, public :: SPRITE_CURVE_DOWN  = 11
    integer, parameter, public :: SPRITE_CURVE_LEFT  = 12
    integer, parameter, public :: SPRITE_STRAIGHT_V  = 13
    integer, parameter, public :: SPRITE_STRAIGHT_HZ = 14
    integer, parameter, public :: SPRITE_FOOD        = 15
    integer, parameter, public :: SPRITE_SURFACE     = 16

    ! Sprite pixels (ARGB).
    type, public :: sprite_type
        integer :: data(SPRITE_SIZE)
    end type sprite_type

    public :: ui_cache
    public :: ui_direction
    public :: ui_draw
    public :: ui_game_over
    public :: ui_load_sprites
    public :: ui_open_window
contains
    function ui_direction(key)
        integer, intent(in) :: key
        integer             :: ui_direction

        ui_direction = DIR_NONE

        select case (key)
            case (PROCALL_KEY_RIGHT)
                ui_direction = DIR_RIGHT

            case (PROCALL_KEY_LEFT)
                ui_direction = DIR_LEFT

            case (PROCALL_KEY_UP)
                ui_direction = DIR_UP

            case (PROCALL_KEY_DOWN)
                ui_direction = DIR_DOWN
        end select
    end function ui_direction

    function ui_load_sprites(file_path, sprites, sprite_size) result(rc)
        character(len=*),  intent(in)    :: file_path
        type(sprite_type), intent(inout) :: sprites(:)
        integer,           intent(in)    :: sprite_size
        character(len=3)                 :: buf
        integer                          :: rc
        integer                          :: i, j, fu

        open (access='stream', action='read', file=file_path, form='unformatted', &
              iostat=rc, newunit=fu, status='old')

        if (rc /= 0) return

        do j = 1, size(sprites)
            do i = 1, sprite_size, 3
                ! Read 3 bytes.
                read (fu) buf

                sprites(j)%data(i)     = transfer(buf(1:1), 0) ! R
                sprites(j)%data(i + 1) = transfer(buf(2:2), 0) ! G
                sprites(j)%data(i + 2) = transfer(buf(3:3), 0) ! B
            end do
        end do

        close (fu)
    end function ui_load_sprites

    function ui_open_window(win, width, height, title, font) result(rc)
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
        call newfontset(win, font // c_null_char, rc)
        if (rc /= 0) return
        rc = 0
    end function ui_open_window

    subroutine ui_cache(win, world, sprites)
        integer,           intent(inout) :: win
        type(world_type),  intent(inout) :: world
        type(sprite_type), intent(inout) :: sprites(:)
        integer                          :: x, y

        call layer(win, 0, 2)

        do y = 0, world%height - 1
            do x = 0, world%width - 1
                call putimg24(win, &
                              real(x * BLOCK_SIZE), &
                              real(y * BLOCK_SIZE), &
                              BLOCK_SIZE, &
                              BLOCK_SIZE, &
                              sprites(SPRITE_SURFACE)%data)
            end do
        end do
    end subroutine ui_cache

    subroutine ui_draw(win, world, body, sprites)
        integer,           intent(inout) :: win
        type(world_type),  intent(inout) :: world
        type(body_type),   intent(inout) :: body
        type(sprite_type), intent(inout) :: sprites(:)
        integer                          :: dx1, dy1, dx2, dy2
        integer                          :: sprite
        type(part_type), pointer         :: ptr

        call layer(win, 0, 1)
        call copylayer(win, 2, 1)

        ! Draw snake.
        ptr => body%head

        do while (associated(ptr))
            if (associated(ptr, body%head)) then
                sprite = SPRITE_HEAD + ptr%dir
            else if (associated(ptr, body%tail)) then
                sprite = SPRITE_TAIL + ptr%dir
            else if (associated(ptr%prev) .and. associated(ptr%next)) then
                dx1 = ptr%x - ptr%prev%x
                dy1 = ptr%y - ptr%prev%y
                dx2 = ptr%x - ptr%next%x
                dy2 = ptr%y - ptr%next%y

                ! Select sprite of segment.
                if (dx1 == 0 .and. dx2 == 0) then
                    sprite = SPRITE_STRAIGHT_V
                else if (dy1 == 0 .and. dy2 == 0) then
                    sprite = SPRITE_STRAIGHT_HZ
                else if ((dx2 == 1 .and. dy1 == -1) .or. (dx1 == 1 .and. dy2 == -1)) then
                    sprite = SPRITE_CURVE_LEFT
                else if ((dx2 == 1 .and. dy1 == 1) .or. (dx1 == 1 .and. dy2 == 1)) then
                    sprite = SPRITE_CURVE_DOWN
                else if ((dx2 == -1 .and. dy1 == 1) .or. (dx1 == -1 .and. dy2 == 1)) then
                    sprite = SPRITE_CURVE_RIGHT
                else if ((dx2 == -1 .and. dy1 == -1) .or. (dx1 == -1 .and. dy2 == -1)) then
                    sprite = SPRITE_CURVE_UP
                end if
            end if

            call putimg24(win, &
                          real(ptr%x * BLOCK_SIZE - 1), &
                          real(ptr%y * BLOCK_SIZE - 1), &
                          BLOCK_SIZE, &
                          BLOCK_SIZE, &
                          sprites(sprite)%data)
            ptr => ptr%next
        end do

        ! Draw food.
        sprite = SPRITE_FOOD
        call putimg24(win, &
                      real(world%food%x * BLOCK_SIZE - 1), &
                      real(world%food%y * BLOCK_SIZE - 1), &
                      BLOCK_SIZE, &
                      BLOCK_SIZE, &
                      sprites(sprite)%data)

        call copylayer(win, 1, 0)
    end subroutine ui_draw

    subroutine ui_game_over(win)
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
            if (key > 0) exit
        end do
    end subroutine ui_game_over
end module ui

program main
    use :: procall
    use :: snake
    use :: ui
    implicit none

    character(len=*), parameter :: FILE_NAME = 'sprites.bin'
    character(len=*), parameter :: FONT_NAME = '-*-helvetica-medium-o-*-*-24-*-*-*-*-*-*-*'

    integer, parameter :: WORLD_WIDTH  = 40
    integer, parameter :: WORLD_HEIGHT = 30
    integer, parameter :: NSPRITES     = 16

    integer, parameter :: WIN_WIDTH  = WORLD_WIDTH * BLOCK_SIZE
    integer, parameter :: WIN_HEIGHT = WORLD_HEIGHT * BLOCK_SIZE
    integer, parameter :: DELAY      = 60

    integer           :: next
    integer           :: dt
    integer           :: key
    integer           :: win
    logical           :: is_reset
    real              :: t1, t2
    type(body_type)   :: body
    type(part_type)   :: head
    type(world_type)  :: world
    type(sprite_type) :: sprites(NSPRITES)

    call random_seed()

    ! Load sprites from file.
    if (ui_load_sprites(FILE_NAME, sprites, SPRITE_SIZE) /= 0) then
        print '(3a)', 'Error: Loading file "', trim(FILE_NAME), '" failed'
        stop
    end if

    ! Open X11 window.
    if (ui_open_window(win, WIN_WIDTH, WIN_HEIGHT, 'Fortran Snake', FONT_NAME) /= 0) then
        print '(3a)', 'Error: Loading font "', trim(FONT_NAME), '" failed'
        call gcloseall()
        stop
    end if

    is_reset = .true.

    loop: do
        call cpu_time(t1)

        ! Initialise game.
        if (is_reset) then
            ! Remove old body.
            call snake_delete(body)

            ! Initialise snake.
            call snake_new(body)
            head = part_type(WORLD_WIDTH / 2, WORLD_HEIGHT / 2, DIR_UP, null(), null())
            call snake_add(body, head)

            ! Initialise world.
            world = world_type(width  = WORLD_WIDTH, &
                               height = WORLD_HEIGHT, &
                               food   = food_type(0, 0))

            ! Place food.
            call snake_new_food(body, world)

            ! Draw world to screen and get initial direction.
            call ui_cache(win, world, sprites)
            call ui_draw(win, world, body, sprites)

            ! Set blocking keyboard input.
            call gsetnonblock(0)

            do
                call ggetch(key)
                if (key == PROCALL_KEY_ESCAPE) exit loop
                body%head%dir = ui_direction(key)
                if (body%head%dir /= DIR_NONE) exit
            end do

            ! Set non-blocking keyboard input.
            call gsetnonblock(1)
            is_reset = .false.
        end if

        ! Get input.
        call ggetch(key)
        if (key == PROCALL_KEY_ESCAPE) exit loop
        next = ui_direction(key)
        if (next /= DIR_NONE) body%head%dir = next

        ! Move snake.
        if (.not. snake_move(body, world)) then
            call ui_game_over(win)
            is_reset = .true.
            cycle
        end if

        ! Draw everything.
        call ui_draw(win, world, body, sprites)

        call cpu_time(t2)
        dt = int((t2 - t1) * 1000)
        if (dt < DELAY) call msleep(DELAY - dt)
    end do loop

    call snake_delete(body)
    call gcloseall()
end program main
