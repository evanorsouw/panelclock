library IEEE;
use IEEE.STD_LOGIC_1164.all;
use IEEE.NUMERIC_STD.all;

entity ledpanel_controller is
   port (    
      i_clk180M     : in std_logic;
      i_reset_n     : in std_logic;
      i_uart_rx     : in std_logic;
      --
      o_dsp_clk     : out std_logic;
      o_dsp_latch   : out std_logic;
      o_dsp_oe      : out std_logic;
      o_dsp_addr    : out std_logic_vector (4 downto 0);
      o_dsp_rgbs    : out std_logic_vector (11 downto 0);
      o_dsp_vbl     : out std_logic;
      -- sram pins
      o_sram_oe     : out std_logic;
      o_sram_wr     : out std_logic;
      o_sram_cs     : out std_logic;
      o_sram_addr   : out std_logic_vector(14 downto 0);
      io_sram_data  : inout std_logic_vector(11 downto 0)
   );
end entity ledpanel_controller;

architecture ledpanel_controller_arch of ledpanel_controller is
  
   component FM6124
   port (    
      i_clk60M    : in std_logic;
      i_reset_n   : in std_logic;
      --
      o_addr      : out std_logic_vector (14 downto 0);
      o_dsp_clk   : out std_logic;
      o_dsp_latch : out std_logic;
      o_dsp_oe    : out std_logic;
      o_dsp_addr  : out std_logic_vector (4 downto 0);
      o_dsp_vbl   : out std_logic
   );
   end component;
 
   component linear2logarithmic
   port (    
      lin    : in std_logic_vector (7 downto 0);
      --
      log    : out std_logic_vector (7 downto 0)
   );
   end component;   

   component UART
   port
   (
      i_clkx8       : in std_logic;
      i_reset_n     : in std_logic;
      i_rx          : in std_logic;
      o_datain      : out std_logic_vector (7 downto 0);
      o_datain_clk  : out std_logic
   );
   end component;
   
   component FIFO
   generic (
      FIFO_DEPTH : integer := 12;  -- FIFO can cache 2^14 = 16K bytes
      FIFO_WIDTH : natural := 8
   );
   port (    
      i_reset_n : in std_logic;
      i_clk     : in std_logic;
      i_wen     : in std_logic;
      i_ren     : in std_logic;
      i_data    : in std_logic_vector(FIFO_WIDTH-1 downto 0);
      o_data    : out std_logic_vector(FIFO_WIDTH-1 downto 0);
      o_full    : out std_logic;
      o_empty   : out std_logic
   );
   end component;
   
   signal s_clk60M           : std_logic;
   signal s_clk24M           : std_logic;
   signal s_uart_datain      : std_logic_vector (7 downto 0);
   signal s_uart_dataclk     : std_logic;
   signal s_ram_rd_addr      : std_logic_vector (14 downto 0);
                           
   signal s_ram_wr_clk       : std_logic;
   signal s_ram_wr_mask      : std_logic_vector(11 downto 0);
   signal s_ram_wr_addr      : unsigned(14 downto 0);
   signal s_ram_wr_bitsR     : std_logic_vector(7 downto 0);
   signal s_ram_wr_bitsG     : std_logic_vector(7 downto 0);
   signal s_ram_wr_bitsB     : std_logic_vector(7 downto 0);
   
   signal s_fifo_empty       : std_logic;
   signal s_fifo_wen         : std_logic;
   signal s_fifo_ren         : std_logic;
   signal s_fifo_dataout     : std_logic_vector (7 downto 0);
   signal s_fifo_datain      : std_logic_vector (7 downto 0);

   signal s_api_colordata    : std_logic_vector (7 downto 0);
   signal s_dsp_latch        : std_logic;

   signal s_rect_x            : unsigned(7 downto 0);
   signal s_rect_px           : unsigned(7 downto 0);
   signal s_rect_py           : unsigned(7 downto 0);
   signal s_rect_dx           : unsigned(7 downto 0);
   signal s_rect_dy           : unsigned(7 downto 0);

   signal s_rmw_step         : integer := 0;
   signal s_rmw_address      : unsigned(14 downto 0) := "000000000000000";
   signal s_rmw_readbits     : std_logic_vector(11 downto 0);
   signal s_rmw_bitmask      : std_logic_vector(7 downto 0);
   signal s_rmw_in_progress  : std_logic := '0';

begin
   LEDChip : FM6124
   port map (
      i_clk60M    => s_clk60M,
      i_reset_n   => i_reset_n,
      --
      o_addr      => s_ram_rd_addr,
      o_dsp_clk   => o_dsp_clk,
      o_dsp_latch => s_dsp_latch,
      o_dsp_addr  => o_dsp_addr,
      o_dsp_oe    => o_dsp_oe,
      o_dsp_vbl   => o_dsp_vbl
   );
        
   color_correct : linear2logarithmic
   port map (
      lin => s_fifo_dataout,
      log => s_api_colordata
   );  
   
   PC : UART
   port map (
      i_clkx8       => s_clk24M,
      i_reset_n     => i_reset_n,
      i_rx          => i_uart_rx,
      --
      o_datain      => s_uart_datain,
      o_datain_clk  => s_uart_dataclk
   );
   
   PCFIFO : FIFO 
   port map (
      i_reset_n => i_reset_n,
      i_clk     => i_clk180M,
      i_wen     => s_fifo_wen,
      i_ren     => s_fifo_ren,
      i_data    => s_fifo_datain,
      o_data    => s_fifo_dataout,
      o_full    => open,
      o_empty   => s_fifo_empty
   );
   
   o_dsp_latch <= s_dsp_latch;
      
   p_clocks: process (i_clk180M)
   variable clk_scaler  : integer := 0;
   variable uart_scaler : integer := 0;
   begin
      if rising_edge(i_clk180M) then
                 
         case clk_scaler is
         when 0 => 
            s_clk60M <= '1';                      
            clk_scaler := 1;
         when 1 => 
            s_clk60M <= '0';
            clk_scaler := 2;
         when 2 => 
            clk_scaler := 0;
         when others =>         
            clk_scaler := 0;
         end case;
         
         -- convert 180MHz to 24MHZ: divide by 7.5 (alternating 8 cycles + 7 cycles)
         --    ___     ___    ___     ___    __
         -- __| 4 |_4_| 4 |_3| 4 |_4_| 4 |_3| 
         -- 11012345678911111012345678911111101
         -- 34          01234          01234
         case uart_scaler is
         when 0 => 
         when 8 => 
            s_clk24M <= '1';
            uart_scaler := uart_scaler + 1;
         when 4 => 
         when 12 => 
            s_clk24M <= '0';
            uart_scaler := uart_scaler + 1;
         when 15 =>
            uart_scaler := 0;
         when others =>         
            uart_scaler := uart_scaler + 1;
         end case;
      end if;
   end process;

   p_sram: process(i_reset_n, i_clk180M)   
   -- display structure:
   --  2 64x64 panels P0, P1 each with an 64x32 upper (0) and lower (1) half.
   --  +-------++-------+
   --  | P0,0  || P1,0  |
   --  +-------++-------+
   --  | P0,1  || P1,1  |
   --  +-------++-------+
   -- Each panel half has 64x32 pixels. Each pixels has 3 RGB leds controlled by 3 bytes.
   -- With a color resolution of 8 bit you get 256 intensities per color.
   -- Each panel half has its own RGB inputs that are clocked in 1 line at-a-time.
   -- Since we have 4 panel halves, we can clock 4x3 bits with every display clock.
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
   -- For keep consistent datastream from memory for displaying the updates, we store
   -- in memory, the 4x3 RGB bits for all 4 panel halves.
   -- This means that each pixel we progress in a line, we also progress 1 memory address.
   -- To display all 32x64 pixels we need 64*8*32=16384 12 bit words.
   -- Each 12bit word represents 1 bit of the color intensities for all 4 64x32 panels: 
   --   [P0,0] [P0,1] [P1,0] [P1,1]
   --    11:9   8:6    5:3    2:0
   --    RGB    RGB    RGB    RGB 
   -- The display is controlled using 8 bit channels (256 intensities) using 
   -- BAM (Bit Angle Modulation).
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
   -- Such an update is initiated by setting the:
   --   s_ram_wr_addr
   --   s_ram_wr_mask
   --   s_ram_wr_clk
   variable v_updated_bits   : std_logic_vector (11 downto 0);
   begin    
      if i_reset_n = '0' then      
         o_sram_cs         <= '1'; 
         s_rmw_in_progress <= '0';
               
      elsif rising_edge(i_clk180M) then      
         o_sram_cs    <= '0';
         o_sram_oe    <= '0';
         
         if s_ram_wr_clk = '1' then -- will remain high only 1 clock
            s_rmw_in_progress  <= '1';
            s_rmw_bitmask      <= "10000000";
            s_rmw_address      <= s_ram_wr_addr;
            s_rmw_step         <= 0;  -- an interrupted read-modify-write cycle is restarted
         end if;
            
         if s_dsp_latch = '0' then           
            -- read pixel values during a row refresh cycle.
            o_sram_wr     <= '1';
            o_sram_addr   <= s_ram_rd_addr;
            io_sram_data  <= (others => 'Z'); 
            o_dsp_rgbs    <= io_sram_data;          
            s_rmw_step    <= 0;  -- a possible interrupted read-modify-write cycle is restarted
            
         elsif s_rmw_in_progress = '1' then
            -- update external display ram during periods where display is idle showing a row.
            s_rmw_step  <= s_rmw_step + 1;     
            case s_rmw_step is
            when 0 | 1 | 2 => -- set address to read pixelbits
               if s_rmw_bitmask = "00000000" then
                  -- all bits written, we're done
                  s_rmw_in_progress   <= '0';
               end if;
               o_sram_addr  <= std_logic_vector(s_rmw_address);
               o_sram_wr    <= '1';
               io_sram_data <= (others => 'Z'); 
            when 3 | 4 => -- read pixelbits
               s_rmw_readbits <= io_sram_data and not s_ram_wr_mask; 
            when 5 | 6 | 7 => -- write modified pixel bits
               v_updated_bits := s_rmw_readbits;
               if ((s_ram_wr_bitsR and s_rmw_bitmask) /= "00000000") then
                  v_updated_bits := v_updated_bits or (s_ram_wr_mask and "001001001001");
               end if;            
               if ((s_ram_wr_bitsG and s_rmw_bitmask) /= "00000000") then
                  v_updated_bits := v_updated_bits or (s_ram_wr_mask and "010010010010");
               end if;            
               if ((s_ram_wr_bitsB and s_rmw_bitmask) /= "00000000") then
                  v_updated_bits := v_updated_bits or (s_ram_wr_mask and "100100100100");
               end if;            
               io_sram_data <= v_updated_bits;
               o_sram_addr <= std_logic_vector(s_rmw_address);
               o_sram_wr   <= '0'; 
            when 8 =>
               -- prepare for next bit
               s_rmw_bitmask  <= '0' & s_rmw_bitmask(7 downto 1);
               s_rmw_address   <= s_rmw_address + 16#0800#;
            when others =>
               s_rmw_step <= 0;
            end case;                   
         end if;
      end if;
   end process;
   
   p_receive_api: process(i_reset_n, i_clk180M)
   variable v_rcv_data         : std_logic_vector (7 downto 0);
   variable v_rcv_dataclk      : std_logic;
   variable v_last_rcv_dataclk : std_logic;
   variable v_init_fill_delay  : integer;
   variable v_init_fill_state  : integer; 
   begin
      if i_reset_n = '0' then
         v_rcv_dataclk      := '0';
         v_last_rcv_dataclk := '0';
         v_init_fill_state  := 0;
         v_init_fill_delay  := 1;
      
      elsif rising_edge(i_clk180M) then      
         if v_init_fill_state = 100 then
            v_rcv_data    := s_uart_datain;
            v_rcv_dataclk := s_uart_dataclk;
         else
            v_init_fill_delay := v_init_fill_delay - 1;
            if v_init_fill_delay = 0 then
               v_init_fill_delay := 2;
               case v_init_fill_state is
               when 0 => v_rcv_data := X"02"; -- fill rect
               when 1 => v_rcv_data := X"00"; -- x
               when 2 => v_rcv_data := X"00"; -- y
               when 3 => v_rcv_data := X"80"; -- dx
               when 4 => v_rcv_data := X"40"; -- dy
               when 5 => v_rcv_data := X"00"; -- R
               when 6 => v_rcv_data := X"00"; -- G
               when 7 => v_rcv_data := X"00"; -- B
               when 8 => v_rcv_data := X"02"; -- fill rect
               when 9 => v_rcv_data := X"02"; -- x
               when 10 => v_rcv_data := X"02"; -- y
               when 11 => v_rcv_data := X"03"; -- dx
               when 12 => v_rcv_data := X"03"; -- dy
               when 13 => v_rcv_data := X"20"; -- R
               when 14 => v_rcv_data := X"20"; -- G
               when 15 => v_rcv_data := X"00"; -- B
               when 16 => v_rcv_data := X"02"; -- fill rect
               when 17 => v_rcv_data := X"05"; -- x
               when 18 => v_rcv_data := X"05"; -- y
               when 19 => v_rcv_data := X"05"; -- dx
               when 20 => v_rcv_data := X"03"; -- dy
               when 21 => v_rcv_data := X"FF"; -- R
               when 22 => v_rcv_data := X"00"; -- G
               when 23 => v_rcv_data := X"00"; -- B
               when others => v_rcv_data := X"00";
               end case;
               v_init_fill_state := v_init_fill_state + 1;
               v_rcv_dataclk := '1';
            else
               v_rcv_dataclk := '0';
            end if;
         end if;
         
         -- write received data into fifo.
         if v_rcv_dataclk = '1' and v_last_rcv_dataclk = '0' then
            s_fifo_datain <= v_rcv_data;
            s_fifo_wen    <= v_rcv_dataclk;
         else
            s_fifo_wen    <= '0';
         end if;
         v_last_rcv_dataclk := v_rcv_dataclk;
      end if;   
   end process;
      
   p_handleapi: process (i_reset_n, i_clk180M)
   type T_APISTATE is ( 
      WAIT_CMD, 
      BLTX, BLTY, BLTDX, BLTDY, BLT_RED, BLT_GRN, BLT_BLU, BLT_PIXEL, BLT_WAIT_PIXEL_START, BLT_WAIT_PIXEL_END, 
      FILLX, FILLY, FILLDX, FILLDY, FILL_RED, FILL_GRN, FILL_BLU, FILL_PIXEL, FILL_WAIT_PIXEL_START, FILL_WAIT_PIXEL_END
   ); 
   type T_READFIFOSTATE is ( PREPARE, GET, EXECUTE ); 
   variable v_readfifostate       : T_READFIFOSTATE;
   variable v_apistate            : T_APISTATE;   
   begin   
      if i_reset_n = '0' then
         s_fifo_ren        <= '0';
         s_ram_wr_clk      <= '0';
         s_ram_wr_mask     <= (others => '0');    
         s_ram_wr_addr     <= (others => '0');               
      
      elsif rising_edge(i_clk180M) then
         -- s_ram_wr_mask indicates that a colorbyte is currently being written (this
         -- takes 8 read-modify-write cycles. It is cleared by process p_sram when completed.
         s_ram_wr_clk <= '0';         
         case v_readfifostate is
         when PREPARE =>
            if s_fifo_empty = '0' then
               s_fifo_ren      <= '1';
               v_readfifostate := GET;
            end if;
         when GET =>
            s_fifo_ren      <= '0';
            v_readfifostate := EXECUTE;
         when EXECUTE =>
            v_readfifostate := PREPARE;
            case v_apistate is
            when WAIT_CMD =>            
               if (unsigned(s_fifo_dataout) = X"01") then
                  v_apistate := BLTX;
               end if;
               if (unsigned(s_fifo_dataout) = X"02") then
                  v_apistate := FILLX;
               end if;
            when BLTX =>
               s_rect_x <= unsigned(s_fifo_dataout(7 downto 0));
               s_rect_px <= unsigned(s_fifo_dataout(7 downto 0));
               v_apistate := BLTY;
            when BLTY =>
               s_rect_py <= unsigned(s_fifo_dataout(7 downto 0));
               v_apistate := BLTDX; 
            when BLTDX =>
               s_rect_dx <= unsigned(s_fifo_dataout(7 downto 0));
               v_apistate := BLTDY;
            when BLTDY =>
               s_rect_dy <= unsigned(s_fifo_dataout(7 downto 0));
               v_apistate := BLT_RED;            
            when BLT_RED =>
               s_ram_wr_bitsR <= s_api_colordata;
               v_apistate := BLT_GRN;
            when BLT_GRN =>
               s_ram_wr_bitsG <= s_api_colordata;
               v_apistate := BLT_BLU;
            when BLT_BLU =>
               s_ram_wr_bitsB <= s_api_colordata;
               v_apistate := BLT_PIXEL;
               v_readfifostate := EXECUTE;
            when BLT_PIXEL =>
               if s_rect_dy = 0 or s_rect_dx = 0 then
                  v_apistate      := WAIT_CMD;
               else
                  s_ram_wr_addr <= "0000" & s_rect_py(4 downto 0) & s_rect_px(5 downto 0);
                  case std_logic_vector'(s_rect_px(6) & s_rect_py(5)) is
                  when "00" =>
                      s_ram_wr_mask <= "000000000111";
                  when "01" =>
                      s_ram_wr_mask <= "000000111000";
                  when "10" =>
                      s_ram_wr_mask <= "000111000000";
                  when "11" =>
                      s_ram_wr_mask <= "111000000000";
                  when others =>
                  end case;
                  s_ram_wr_clk    <= '1';
                  v_apistate      := BLT_WAIT_PIXEL_START;
                  v_readfifostate := EXECUTE;
                  -- prepare next pixel
                  s_rect_px <= s_rect_px + 1;
                  if s_rect_px = s_rect_x + s_rect_dx - 1 then
                     s_rect_px <= s_rect_x;
                     s_rect_py <= s_rect_py + 1;
                     s_rect_dy <= s_rect_dy - 1;
                  end if;                                     
               end if;
            when BLT_WAIT_PIXEL_START =>
               if s_rmw_in_progress = '1' then
                  v_apistate := BLT_WAIT_PIXEL_END;
                  v_readfifostate := EXECUTE;
               end if;
            when BLT_WAIT_PIXEL_END =>
               if s_rmw_in_progress = '0' then
                  v_apistate := BLT_RED;
               end if;
            when FILLX =>
               s_rect_x <= unsigned(s_fifo_dataout(7 downto 0));
               s_rect_px <= unsigned(s_fifo_dataout(7 downto 0));
               v_apistate := FILLY;
            when FILLY =>
               s_rect_py <= unsigned(s_fifo_dataout(7 downto 0));
               v_apistate := FILLDX; 
            when FILLDX =>
               s_rect_dx <= unsigned(s_fifo_dataout(7 downto 0));
               v_apistate := FILLDY;
            when FILLDY =>
               s_rect_dy <= unsigned(s_fifo_dataout(7 downto 0));
               v_apistate := FILL_RED;            
            when FILL_RED =>
               s_ram_wr_bitsR <= s_api_colordata;
               v_apistate := FILL_GRN;
            when FILL_GRN =>
               s_ram_wr_bitsG <= s_api_colordata;
               v_apistate := FILL_BLU;
            when FILL_BLU =>
               s_ram_wr_bitsB <= s_api_colordata;
               v_apistate := FILL_PIXEL;
               v_readfifostate := EXECUTE;
            when FILL_PIXEL =>
               if s_rect_dy = 0 or s_rect_dx = 0 then
                  v_apistate := WAIT_CMD;
               else
                  s_ram_wr_addr <= "0000" & s_rect_py(4 downto 0) & s_rect_px(5 downto 0);
                  case std_logic_vector'(s_rect_px(6) & s_rect_py(5)) is
                  when "00" =>
                      s_ram_wr_mask <= "000000000111";
                  when "01" =>
                      s_ram_wr_mask <= "000000111000";
                  when "10" =>
                      s_ram_wr_mask <= "000111000000";
                  when "11" =>
                      s_ram_wr_mask <= "111000000000";
                  when others =>
                  end case;
                  s_ram_wr_clk    <= '1';
                  v_apistate      := FILL_WAIT_PIXEL_START;
                  v_readfifostate := EXECUTE;
                  -- prepare next pixel
                  s_rect_px <= s_rect_px + 1;
                  if s_rect_px = s_rect_x + s_rect_dx - 1 then
                     s_rect_px <= s_rect_x;
                     s_rect_py <= s_rect_py + 1;
                     s_rect_dy <= s_rect_dy - 1;
                  end if;                                     
               end if;
            when FILL_WAIT_PIXEL_START =>
               if s_rmw_in_progress = '1' then
                  v_apistate := FILL_WAIT_PIXEL_END;
               end if;
               v_readfifostate := EXECUTE;
            when FILL_WAIT_PIXEL_END =>
               if s_rmw_in_progress = '0' then
                  v_apistate := FILL_PIXEL;
               end if;
               v_readfifostate := EXECUTE;
            end case;      
         end case;   
      end if;      
   end process;
   
end architecture ledpanel_controller_arch;            