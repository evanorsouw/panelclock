library IEEE;
use IEEE.STD_LOGIC_1164.all;
use IEEE.NUMERIC_STD.all;

-- Internals:
-- This code uses a theoretical maximum of 4x4 panels. The restriction is in the
-- pixel-drawing coordinates that are only a byte so restricted to reach 4 panels.
-- The current code supports only 2 64x64 panels, this is in the fact that all RGB
-- bits are handled as a 12 bit array (explained later). Todo: make #panels configurable.
--
--    +----+----+----+----+
--    | P0 | P1 | P2 | P3 |
--    |    |    |    |    |
--    +----+----+----+----+
--    | P4 | P5 | P6 | P7 |
--    |    |    |    |    |
--    +----+----+----+----+
--    | P8 | P9 | P  | P  |
--    |    |    | 10 | 11 |
--    +----+----+----+----+
--    | P  | P  | P  | P  |
--    | 12 | 13 | 14 | 14 |
--    +----+----+----+----+
--
-- Via the HUB75E interface that these panels provide you continuously have to shift in
-- pixels 1 row at a time. A pixel is either on or off, there is no native intensity
-- available. Intensity is achieved by repeatedly shifting in bits into the panel
-- to achieve a PWM signal per pixel where the on/off ratio determines the intensity.
-- Obviously more intensities mean more lower refreshrate.
-- 
-- Panels are in essence section of 64x32 pixels that need to be refreshed continuously
-- if a panel is 64x64 or if you have more panels, then the looping through of 64x32
-- pixels is done once but, RGB values are output for each 64x32 section indivudually.
-- Now 1 additional note here, panels can be chained. For this each panel has a HUB75E 
-- input connector and an output connector. Data shifted in is also shifted out on
-- the output connector (after 64 bits). Chaining means simpler hardware but since 
-- chaining means that each row becomes longer, the effective refresh rate will again drop. 
-- In the following description it is assumed that the underlying hardware provides
-- a HUB75E connector for each panel.
-- 
-- In a nutshell to integrate this code with external memory;
-- The address width is 64*32*8 = 14 bits, regardless the number of panels.
-- The data width = 3bits per 64x32 panel. So 2 64x64 panels have a datawidth of 2x2x3 = 12 bits.
-- When double buffering of more screen is desired, increases the address space.
-- 15 address bits will result in 2 screens, 16 address bits in 4 screens etc.
--
-- P0, P1 each with an 64x32 upper (U) and lower (L) half.
--  +-------+ +-------+
--  |  P0U  | |  P1L  |
--  +-------+ +-------+
--  |  P0U  | |  P1L  |
--  +-------+ +-------+
-- Each panel half has 64x32 pixels. Each pixels has 3 RGB leds controlled by 3 bytes.
-- With a color resolution of 8 bit you get 256 intensities per color.
-- Each panel half has its own RGB inputs that are clocked in 1 line at-a-time.
-- For ach panels half we clock in 3 bits with every display clock.
-- We use BAM (Bit Angle Modulation) to generate a PWM signal in 8 steps with
-- decreasing intervals: bit7:128/256  bit6:64/256, bit5:32/256 ... bit0:1/256
-- The duration for clocking the line corresponding to bit0 take 64clks. We add
-- to this 3 clks overhead (Latch, OE).
-- To clock the entire display of 32 lines we need 32*67 clocks
-- To clock in the entire display with 8 bits resolution (256 itensities) we have
-- 256*32*67 clocks.
-- With a max clock of 30MHz this means a single screen renders in (256*32*67)/30E6 = 18.3ms.
-- This means a refreshrate of 54.6 Hz.
-- In practice we clock in bit7 for all 32 lines, following by bit6 for all 32 lines etc.
-- For keep consistent datastream from memory for displaying the updates, we have the 
-- datasize for each address equal to the required RGB bits for all panel halves.
-- This means that each pixel we progress 1 pixel in a line, we also progress 1 memory address.
-- To display all 32x64 pixels we need 64*32*8=16384 words.
-- How the individual bits of each word maps to a panel is implementation defined and based 
-- on the available datasize. If we want 4 64x64 panels we need 4*2*3 = 24 bit wordsize
-- in terms of ram, this means 3 parallel 8bit ram chips.
-- 
-- Note that HUB75 panels can be chained. Here we only clock in 1 panel (64 pixels).
-- Each 64x64 panel get its own HUB75 output where each HUB75 connector gets its own
-- RGBRGB values but they all share the Clk,Latch,OE and  ABCDE lines. 
-- A chained version for 2 64x64 would require only 1 HUB75 connector but each line
-- now take 128 clock cycles. This means that the refresh rate effectively is halved.
-- We could counteract that by reducing the color resulotion from 8 to 7 bits.
--
-- memory addresses:
-- @0000 -> bit7 @0,0      (bits 7 shown for 128 time units)
-- @0001 -> bit7 @1,0
--  ...
-- @003F -> bit7 @63,0
-- @0040 -> bit7 @0,1
--  ...
-- @07FF -> bit7 @63,31
-- @0800 -> bit6 @0,0      (bits 6 shown for 64 time units)
--  ...
-- @0FFF -> bit6 @63,31
-- @1000 -> bit5 @0,0      (bits 5 shown for 32 time units)
-- @1800 -> bit4 @0,0      (bits 4 shown for 16 time units)
-- @2000 -> bit3 @0,0      (bits 3 shown for 8 time units)
-- @2800 -> bit2 @0,0      (bits 2 shown for 4 time units)
-- @3000 -> bit1 @0,0      (bits 1 shown for 2 time units)
-- @3800 -> bit0 @0,0      (bits 0 shown for 1 time unit)
-- So a read-modify-write cycle of a single pixel involves 8 updates in memory.
-- The adressspace needed to display a single 64x32 panel half is 0x4000 (16K) or 14 bits.
-- with a datawidth of 3 bits.
--
-- To support double-buffering of screen we a additional address bit can be introduced for
-- both displaying and writing. by toggling these bits on the start of displaying a new
-- screen we can write the display without visual artifacts.
-- currently trhe code supports 4 bits meaning 16 screens.
--

entity ledpanel_controller is
   port (
      i_reset_n       : in std_logic;
      i_clk60M        : in std_logic;
      i_spi_clk       : in std_logic;
      i_spi_sdi       : in std_logic;
      --
      o_spi_sdo       : out std_logic;
      o_dsp_clk       : out std_logic;
      o_dsp_latch     : out std_logic;
      o_dsp_oe_n      : out std_logic;
      o_dsp_addr      : out std_logic_vector (4 downto 0);
      o_dsp_rgbs      : out std_logic_vector (11 downto 0);
      o_dsp_vbl       : out std_logic;
      -- sram pins
      o_sram_oe       : out std_logic;
      o_sram_wr       : out std_logic;
      o_sram_addr     : out std_logic_vector(17 downto 0);   -- 13 bits for 1 screen, use 17 for 16 screens
      o_sram_cs       : out std_logic;
      io_sram_data    : inout std_logic_vector(11 downto 0);
      --
      ot_test         : out std_logic
   );
end entity ledpanel_controller;

architecture ledpanel_controller_arch of ledpanel_controller is

   component reset_controller
   port (
      i_clk              : in std_logic;
      i_external_reset_n : in std_logic;
      --
      o_reset_n          : out std_logic
   );
   end component;

   component FM6124
   port (
      i_clkx2     : in std_logic;
      i_reset_n   : in std_logic;
      --
      o_addr      : out std_logic_vector (13 downto 0);
      o_read      : out std_logic;
      o_dsp_clk   : out std_logic;
      o_dsp_latch : out std_logic;
      o_dsp_oe_n  : out std_logic;
      o_dsp_addr  : out std_logic_vector (4 downto 0);
      o_dsp_vbl   : out std_logic
   );
   end component;

   component SPI
   port
   (
      i_reset_n     : in std_logic;
      i_clk         : in std_logic;
      i_spi_clk     : in std_logic;
      i_spi_sdi     : in std_logic;
      --
      o_datain      : out std_logic_vector (7 downto 0);
      o_datain_clk  : out std_logic;
      o_test        : out std_logic
   );
   end component;

   component whitemagic_init_screen
   port (
      i_idx   : in  unsigned(15 downto 0);
      o_count : out unsigned(7 downto 0);
      o_data  : out std_logic_vector(7 downto 0)
   );
   end component;

   component port2_ram
   generic (
      DEPTH : natural;
      WIDTH : natural
   );
   port (
     i_rclk     : in  std_logic;
     i_raddr    : in  std_logic_vector;
     o_rdata    : out std_logic_vector;
     i_wclk     : in  std_logic;
     i_waddr    : in  std_logic_vector;
     i_wdata    : in  std_logic_vector;
     i_wren     : in  std_logic
   );

   end component;
   component cmd_fill
   port (
      i_reset_n        : in std_logic;
      i_clk            : in std_logic;
      i_data           : in std_logic_vector(7 downto 0);
      i_data_color     : in std_logic_vector(7 downto 0);
      i_data_rdy       : in std_logic;
      i_writing        : in std_logic;
      --
      o_busy           : out std_logic;
      o_need_more_data : out std_logic;
      o_address        : out unsigned(15 downto 0);
      o_rgb            : out std_logic_vector(23 downto 0);
      o_write_clk      : out std_logic
   );
   end component cmd_fill;

   component cmd_blit
   port (
      i_reset_n        : in std_logic;
      i_clk            : in std_logic;
      i_data           : in std_logic_vector(7 downto 0);
      i_data_color     : in std_logic_vector(7 downto 0);
      i_data_rdy       : in std_logic;
      i_writing        : in std_logic;
      --
      o_busy           : out std_logic;
      o_need_more_data : out std_logic;
      o_address        : out unsigned(15 downto 0);
      o_rgb            : out std_logic_vector(23 downto 0);
      o_write_clk      : out std_logic
   );
   end component cmd_blit;

   component cmd_lut
   port (
      i_reset_n        : in std_logic;
      i_clk            : in std_logic;
      i_data           : in std_logic_vector(7 downto 0);
      i_data_rdy       : in std_logic;
      --
      o_busy           : out std_logic;
      o_need_more_data : out std_logic;
      o_lut_waddr      : out std_logic_vector(7 downto 0);
      o_lut_wdata      : out std_logic_vector(7 downto 0);
      o_lut_wren       : out std_logic
   );
   end component;

   component cmd_screen
   port (
      i_reset_n        : in std_logic;
      i_clk            : in std_logic;
      i_data           : in std_logic_vector(7 downto 0);
      i_data_rdy       : in std_logic; -- indication that new data is available
      --
      o_busy           : out std_logic; -- indication that command is in progress
      o_need_more_data : out std_logic; -- indication that more data is needed
      o_displayscreen  : out std_logic_vector(3 downto 0);
      o_writescreen    : out std_logic_vector(3 downto 0)
   );
   end component;

   constant FIFO_DEPTH       : natural := 13;

   signal s_serial_datain    : std_logic_vector (7 downto 0);
   signal s_serial_dataclk   : std_logic;
   signal s_ram_rd_addr      : std_logic_vector (13 downto 0);
   signal s_ram_read         : std_logic;

   signal s_reset_n          : std_logic;

   signal s_init_data_idx    : unsigned(15 downto 0);
   signal s_init_data_size   : unsigned(7 downto 0);
   signal s_init_data_out    : std_logic_vector(7 downto 0);

   signal s_ram_wr_clk       : std_logic;
   signal s_ram_wr_mask      : std_logic_vector(11 downto 0);
   signal s_ram_wr_addr      : unsigned(13 downto 0);
   signal s_ram_wr_color     : std_logic_vector(23 downto 0);

   signal s_fifo_wren        : std_logic;
   signal s_fifo_rindex      : unsigned(FIFO_DEPTH-1 downto 0);
   signal s_fifo_windex      : unsigned(FIFO_DEPTH-1 downto 0);
   signal s_fifo_dataout     : std_logic_vector (7 downto 0);
   signal s_fifo_datain      : std_logic_vector (7 downto 0);
   signal s_received_data    : std_logic_vector (7 downto 0);
   signal s_received_color   : std_logic_vector (7 downto 0);

   signal s_api_colordata    : std_logic_vector (7 downto 0);
   signal s_dsp_clk          : std_logic;
   signal s_dsp_latch        : std_logic;
   signal s_dsp_oe_n         : std_logic;
   signal s_dsp_addr         : std_logic_vector(4 downto 0);
   signal s_dsp_vbl          : std_logic;

   signal s_rmw_step         : integer;
   signal s_rmw_address      : unsigned(13 downto 0);
   signal s_rmw_bitmask      : std_logic_vector(7 downto 0);
   signal s_rmw_in_progress  : std_logic;

   type T_FIFOSTATE is ( REQUEST, LUT, AVAILABLE );
   signal s_readfifo_state : T_FIFOSTATE;
   

   signal s_cmd_fill_data_rdy       : std_logic;
   signal s_cmd_fill_busy           : std_logic;
   signal s_cmd_fill_need_more_data : std_logic;
   signal s_cmd_fill_adress         : unsigned(15 downto 0);
   signal s_cmd_fill_rgb            : std_logic_vector(23 downto 0);
   signal s_cmd_fill_write_clk      : std_logic;

   signal s_cmd_blit_data_rdy       : std_logic;
   signal s_cmd_blit_busy           : std_logic;
   signal s_cmd_blit_need_more_data : std_logic;
   signal s_cmd_blit_adress         : unsigned(15 downto 0);
   signal s_cmd_blit_rgb            : std_logic_vector(23 downto 0);
   signal s_cmd_blit_write_clk      : std_logic;

   signal s_cmd_lut_data_rdy        : std_logic;
   signal s_cmd_lut_busy            : std_logic;
   signal s_cmd_lut_need_more_data  : std_logic;

   signal s_lut_waddr               : std_logic_vector(7 downto 0);
   signal s_lut_wdata               : std_logic_vector(7 downto 0);
   signal s_lut_wren                : std_logic;

   signal s_cmd_screen_data_rdy     : std_logic;
   signal s_cmd_screen_busy         : std_logic;
   signal s_cmd_screen_need_more_data : std_logic;
   signal s_displayscreen           : std_logic_vector(3 downto 0);
   signal s_nextdisplayscreen       : std_logic_vector(3 downto 0);
   signal s_writescreen             : std_logic_vector(3 downto 0);

   signal s_last_rcv_dataclk        : std_logic;
   signal s_rcv_dataclk             : std_logic;
   signal s_init_delay              : unsigned(28 downto 0);
   signal s_init_idx                : unsigned(15 downto 0);
   signal s_init_done               : std_logic;

begin
  reset : reset_controller
  port map (
      i_clk              => i_clk60M,
      i_external_reset_n => i_reset_n,
      --
      o_reset_n          => s_reset_n
   );

   LEDChip : FM6124
   port map (
      i_clkx2     => i_clk60M,
      i_reset_n   => s_reset_n,
      --
      o_addr      => s_ram_rd_addr,
      o_read      => s_ram_read,
      o_dsp_clk   => s_dsp_clk,
      o_dsp_latch => s_dsp_latch,
      o_dsp_addr  => s_dsp_addr,
      o_dsp_oe_n  => s_dsp_oe_n,
      o_dsp_vbl   => s_dsp_vbl
   );

   CPU : SPI
   port map (
      i_reset_n     => s_reset_n,
      i_clk         => i_clk60M,
      i_spi_clk     => i_spi_clk,
      i_spi_sdi     => i_spi_sdi,
      --
      o_datain      => s_serial_datain,
      o_datain_clk  => s_serial_dataclk,
      o_test        => open
   );

   init_data : whitemagic_init_screen
   port map (
      i_idx   => s_init_data_idx,
      o_count => s_init_data_size,
      o_data  => s_init_data_out
   );

   fiforam : port2_ram
   generic map (
      DEPTH => FIFO_DEPTH,
      WIDTH => 8
   )
   port map(
     i_rclk     => i_clk60M,
     i_raddr    => std_logic_vector(s_fifo_rindex),
     o_rdata    => s_fifo_dataout,
     i_wclk     => i_clk60M,
     i_waddr    => std_logic_vector(s_fifo_windex),
     i_wdata    => s_fifo_datain,
     i_wren     => s_fifo_wren
   );

   color_lut : port2_ram
   generic map (
      DEPTH => 8,
      WIDTH => 8
   )
   port map(
     i_rclk     => i_clk60M,
     i_raddr    => s_received_data,
     o_rdata    => s_received_color,
     i_wclk     => i_clk60M,
     i_wdata    => s_lut_wdata,
     i_waddr    => s_lut_waddr,
     i_wren     => s_lut_wren
   );

   cmd_fill_impl : cmd_fill
   port map (
      i_reset_n        => s_reset_n,
      i_clk            => i_clk60M,
      i_data           => s_received_data,
      i_data_color     => s_received_color,
      i_data_rdy       => s_cmd_fill_data_rdy,
      i_writing        => s_rmw_in_progress,
      o_busy           => s_cmd_fill_busy,
      o_need_more_data => s_cmd_fill_need_more_data,
      o_address        => s_cmd_fill_adress,
      o_rgb            => s_cmd_fill_rgb,
      o_write_clk      => s_cmd_fill_write_clk
   );

   cmd_blit_impl : cmd_blit
   port map (
      i_reset_n        => s_reset_n,
      i_clk            => i_clk60M,
      i_data           => s_received_data,
      i_data_color     => s_received_color,
      i_data_rdy       => s_cmd_blit_data_rdy,
      i_writing        => s_rmw_in_progress,
      o_busy           => s_cmd_blit_busy,
      o_need_more_data => s_cmd_blit_need_more_data,
      o_address        => s_cmd_blit_adress,
      o_rgb            => s_cmd_blit_rgb,
      o_write_clk      => s_cmd_blit_write_clk
   );

   cmd_lut_impl : cmd_lut
   port map (
      i_reset_n        => s_reset_n,
      i_clk            => i_clk60M,
      i_data           => s_received_data,
      i_data_rdy       => s_cmd_lut_data_rdy,
      o_busy           => s_cmd_lut_busy,
      o_need_more_data => s_cmd_lut_need_more_data,
      o_lut_waddr      => s_lut_waddr,
      o_lut_wdata      => s_lut_wdata,
      o_lut_wren       => s_lut_wren
   );

   cmd_screen_impl : cmd_screen
   port map (
      i_reset_n        => s_reset_n,
      i_clk            => i_clk60M,
      i_data           => s_received_data,
      i_data_rdy       => s_cmd_screen_data_rdy,
      o_busy           => s_cmd_screen_busy,
      o_need_more_data => s_cmd_screen_need_more_data,
      o_displayscreen  => s_nextdisplayscreen,
      o_writescreen    => s_writescreen
   );

   o_spi_sdo <= i_spi_sdi;

   p_sram: process(s_reset_n, i_clk60M)
   -- An memory write update is initiated by setting the:
   --   s_ram_wr_addr
   --   s_ram_wr_color => contains new 24 bit RGB color
   --   s_ram_wr_mask  => contains the RGB bitmask for the selected panel.
   --   s_ram_wr_clk
   -- set 
   variable v_updated_bits    : std_logic_vector (11 downto 0);  
   begin
      if s_reset_n = '0' then
         o_sram_cs         <= '1';
         s_rmw_in_progress <= '0';

      else
         o_sram_cs <= '0';
         o_sram_oe <= '0';

         if rising_edge(i_clk60M) then

            o_dsp_clk <= s_dsp_clk;
            o_dsp_latch <= s_dsp_latch;
            o_dsp_oe_n <= s_dsp_oe_n;
            o_dsp_addr <= s_dsp_addr;
            o_dsp_vbl <= s_dsp_vbl;

            if s_dsp_vbl = '1' then
               s_displayscreen <= s_nextdisplayscreen;
            end if;

            if s_ram_read = '1' then
               -- read pixel values during a row refresh cycle.
               o_sram_wr <= '1';
               o_sram_addr <= s_displayscreen & s_ram_rd_addr;
               io_sram_data <= (others => 'Z');
               o_dsp_rgbs <= io_sram_data;
               s_rmw_step <= 0;

            else
               -- update external display ram during periods where display is idle showing a row.                 
               if s_rmw_in_progress = '1' then
                  s_rmw_step <= s_rmw_step + 1;
                  case s_rmw_step is
                  when 0 =>  -- write cycle
                     o_sram_addr  <= s_writescreen & std_logic_vector(s_rmw_address);
                     o_sram_wr    <= '1';
                     io_sram_data <= (others => 'Z');
                  when 1 => -- write modified pixel bits
                     v_updated_bits := io_sram_data and not s_ram_wr_mask;
                     if ((s_ram_wr_color(23 downto 16) and s_rmw_bitmask) /= "00000000") then
                        v_updated_bits := v_updated_bits or (s_ram_wr_mask and "100100100100");
                     end if;
                     if ((s_ram_wr_color(15 downto 8) and s_rmw_bitmask) /= "00000000") then
                        v_updated_bits := v_updated_bits or (s_ram_wr_mask and "010010010010");
                     end if;
                     if ((s_ram_wr_color(7 downto 0) and s_rmw_bitmask) /= "00000000") then
                        v_updated_bits := v_updated_bits or (s_ram_wr_mask and "001001001001");
                     end if;
                     io_sram_data <= v_updated_bits;
                     o_sram_addr <= s_writescreen & std_logic_vector(s_rmw_address);
                     o_sram_wr <= '0';
                  when 2 =>
                     -- prepare for next bit
                     s_rmw_bitmask <= '0' & s_rmw_bitmask(7 downto 1);
                     s_rmw_address <= s_rmw_address + 16#0800#;
                     if s_rmw_bitmask /= "00000001" then
                        s_rmw_step <= 0;  -- next bit
                     else                        
                        s_rmw_in_progress <= '0';  -- all bits written, we're done
                     end if;
                  when others =>
                  end case;
               elsif s_ram_wr_clk = '1' then
                  s_rmw_in_progress <= '1';
                  s_rmw_bitmask <= "10000000";
                  s_rmw_address <= s_ram_wr_addr;
                  s_rmw_step <= 0;
               -- else
                  -- s_rmw_in_progress <= '1';
                  -- s_rmw_bitmask <= "10000000";
                  -- s_rmw_address <= "11110000000000";
                  -- s_ram_wr_color <= "111111110000000011110000";
                  -- s_rmw_step <= 0;
               end if;
            end if;
         end if;
      end if;
   end process;

   ot_test <= s_reset_n;   

   -- receive API date (byte), initially from internal storage (init_data)
   -- after that from the external serial source.
   -- received data is stored in a FIFO to decouple reception from processing.
   p_api: process(s_reset_n, i_clk60M)
   variable v_halveselect      : unsigned(1 downto 0);
   variable v_cmd_address      : unsigned(15 downto 0);
   variable v_cmd_rgbs         : std_logic_vector(23 downto 0);
   variable v_cmd_write        : std_logic;
   variable v_init_idx         : unsigned(15 downto 0);
   begin
      if s_reset_n = '0' then
         s_rcv_dataclk      <= '0';
         s_last_rcv_dataclk <= '0';
         s_init_idx         <= to_unsigned(0, s_init_idx'length);
         s_init_delay       <= to_unsigned(2, s_init_delay'length);
         s_init_done        <= '0';

         s_ram_wr_clk      <= '0';
         s_ram_wr_mask     <= (others => '0');
         s_ram_wr_addr     <= (others => '0');
         s_readfifo_state  <= REQUEST;

         s_fifo_rindex     <= to_unsigned(0,FIFO_DEPTH);
         s_fifo_windex     <= to_unsigned(0,FIFO_DEPTH);

      elsif rising_edge(i_clk60M) then

         -- receive serial data (initially 'receive' initialization data)
         if s_init_done = '1' then
            s_fifo_datain <= s_serial_datain;
            s_rcv_dataclk <= s_serial_dataclk;
         else
            s_init_data_idx <= s_init_idx;
            s_fifo_datain <= s_init_data_out;

            s_init_delay <= s_init_delay - 1;
            if s_init_delay = 0 then
                s_init_delay <= to_unsigned(5, s_init_delay'length);
                v_init_idx := s_init_idx + 1;                
                s_init_idx <= v_init_idx;
                if v_init_idx = s_init_data_size then
                    s_init_done <= '1';
                end if;
                s_rcv_dataclk <= '1';
            else
                s_rcv_dataclk <= '0';
            end if;
         end if;

         -- write received data into fifo to decouple from execution.
         if s_rcv_dataclk = '1' and s_last_rcv_dataclk = '0' then
            s_fifo_wren <= '1';
         elsif s_rcv_dataclk = '0' and s_last_rcv_dataclk = '1' then
            s_fifo_windex <= s_fifo_windex + 1;  -- auto wraparound due to power of 2
            s_fifo_wren <= '0';
         end if;
         s_last_rcv_dataclk <= s_rcv_dataclk;


         -- read received data from fifo
         case s_readfifo_state is
         when REQUEST =>
            if s_fifo_windex /= s_fifo_rindex then  -- we have something in the fifo?
               s_fifo_rindex <= s_fifo_rindex + 1;
               s_received_data <= s_fifo_dataout;
               s_readfifo_state <= LUT;
            end if;
         when LUT => -- 1 cycle to do the color lut lookup
            s_readfifo_state <= AVAILABLE;
         when others =>
         end case;

         s_ram_wr_clk <= '0';
         s_cmd_fill_data_rdy <= '0';
         s_cmd_blit_data_rdy <= '0';
         s_cmd_lut_data_rdy <= '0';
         s_cmd_screen_data_rdy <= '0';

         if s_cmd_fill_busy = '1' or
            s_cmd_blit_busy = '1' or
            s_cmd_lut_busy = '1' or
            s_cmd_screen_busy = '1'
         then
            if s_cmd_fill_need_more_data = '1' or
               s_cmd_blit_need_more_data = '1' or
               s_cmd_lut_need_more_data = '1' or
               s_cmd_screen_need_more_data = '1' 
            then
               if s_readfifo_state = AVAILABLE then
                  s_cmd_fill_data_rdy <= s_cmd_fill_need_more_data;
                  s_cmd_blit_data_rdy <= s_cmd_blit_need_more_data;
                  s_cmd_lut_data_rdy <= s_cmd_lut_need_more_data;
                  s_cmd_screen_data_rdy <= s_cmd_screen_need_more_data;
                  s_readfifo_state <= REQUEST;
               end if;
            end if;

            v_cmd_write := '0';
            if s_cmd_fill_write_clk = '1' then
               v_cmd_address := s_cmd_fill_adress;
               v_cmd_rgbs := s_cmd_fill_rgb;
               v_cmd_write := '1';
            elsif s_cmd_blit_write_clk = '1' then
               v_cmd_address := s_cmd_blit_adress;
               v_cmd_rgbs := s_cmd_blit_rgb;
               v_cmd_write := '1';
            end if;

            if v_cmd_write = '1' then
               s_ram_wr_addr <= "000" & v_cmd_address(12 downto 8) & v_cmd_address(5 downto 0);
               s_ram_wr_color <= v_cmd_rgbs;
               v_halveselect(1) := v_cmd_address(13);
               v_halveselect(0) := v_cmd_address(6);
               case v_halveselect is
               when "00" => s_ram_wr_mask <= "000000000111";  -- panel1 top
               when "10" => s_ram_wr_mask <= "000000111000";  -- panel1 bottom
               when "01" => s_ram_wr_mask <= "000111000000";  -- panel2 top
               when "11" => s_ram_wr_mask <= "111000000000";  -- panel2 bottom
               when others =>
               end case;
               s_ram_wr_clk <= '1';    -- initiate a RGB pixel write cycle
            end if;

         else
            if s_readfifo_state = AVAILABLE then
               s_cmd_fill_data_rdy <= '1';
               s_cmd_blit_data_rdy <= '1';
               s_cmd_lut_data_rdy <= '1';
               s_cmd_screen_data_rdy <= '1';
               s_readfifo_state <= REQUEST;
            end if;
         end if;
      end if;
   end process;

end architecture ledpanel_controller_arch;