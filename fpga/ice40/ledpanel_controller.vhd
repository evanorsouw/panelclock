library IEEE;
use IEEE.STD_LOGIC_1164.all;
use IEEE.NUMERIC_STD.all;

entity ledpanel_controller is
   port (    
      clk180M     : in std_logic;
      reset       : in std_logic;
      uart_rx     : in std_logic;
      --
      dsp_clk     : out std_logic;
      dsp_latch   : out std_logic;
      dsp_oe      : out std_logic;
      dsp_addr    : out std_logic_vector (4 downto 0);
      dsp_rgbs    : out std_logic_vector (11 downto 0);
      dsp_vbl     : out std_logic;
      -- sram pins
      sram_oe     : out std_logic;
      sram_wr     : out std_logic;
      sram_cs     : out std_logic;
      sram_addr   : out std_logic_vector(14 downto 0);
      sram_data   : inout std_logic_vector(11 downto 0);
      -- test
      tst_fifo_ren     : out std_logic;
      tst_fifo_dataout : out std_logic_vector(7 downto 0);
      tst_fifo_wen     : out std_logic;
      tst_ram_wr_clk   : out std_logic;
      tst_ram_wr_data  : out std_logic_vector(7 downto 0);
      tst_ram_wr_mask  : out std_logic_vector(11 downto 0);
      tst_color_update_done : out std_logic
   );
end entity ledpanel_controller;

architecture ledpanel_controller_arch of ledpanel_controller is
  
   component FM6124
   port (    
      clk60M    : in std_logic;
      reset     : in std_logic;
      --
      addr      : out std_logic_vector (14 downto 0);
      dsp_clk   : out std_logic;
      dsp_latch : out std_logic;
      dsp_oe    : out std_logic;
      dsp_addr  : out std_logic_vector (4 downto 0);
      vbl       : out std_logic
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
      clkx8         : in std_logic;
      reset         : in std_logic;
      rx            : in std_logic;
      datain        : out std_logic_vector (7 downto 0);
      datain_clk    : out std_logic
   );
   end component;
   
   component FIFO
   generic (
      FIFO_DEPTH : integer := 12;  -- FIFO can cache 2^12 = 4K bytes
      FIFO_WIDTH : natural := 8
   );
   port (    
      i_reset   : in std_logic;
      i_clk     : in std_logic;
      i_wen     : in std_logic;
      i_ren     : in std_logic;
      i_data    : in std_logic_vector(FIFO_WIDTH-1 downto 0);
      o_data    : out std_logic_vector(FIFO_WIDTH-1 downto 0);
      o_full    : out std_logic;
      o_empty   : out std_logic
   );
   end component;
   
   signal panel_reset      : std_logic;
   signal clk60M           : std_logic;
   signal clk24M           : std_logic;
   signal uart_datain      : std_logic_vector (7 downto 0);
   signal uart_dataclk     : std_logic;
   signal ram_rd_addr      : std_logic_vector (14 downto 0);
   signal ram_rd_data      : std_logic_vector (11 downto 0);
                           
   signal ram_wr_clk       : std_logic;
   signal ram_wr_mask      : unsigned(11 downto 0);
   signal ram_wr_addr      : unsigned(14 downto 0);
   signal ram_wr_colorbits : std_logic_vector(7 downto 0);
   signal color_update_done  : std_logic;
   
   signal fifo_empty       : std_logic;
   signal fifo_wen         : std_logic;
   signal fifo_ren         : std_logic;
   signal fifo_datain      : std_logic_vector (7 downto 0);
   signal fifo_dataout     : std_logic_vector (7 downto 0);

   signal api_colordata    : std_logic_vector (7 downto 0);
   signal s_dsp_latch      : std_logic;

begin
   LEDChip : FM6124
   port map (
      clk60M    => clk60M,
      reset     => panel_reset,
      --
      addr      => ram_rd_addr,
      dsp_clk   => dsp_clk,
      dsp_latch => s_dsp_latch,
      dsp_addr  => dsp_addr,
      dsp_oe    => dsp_oe,
      vbl       => dsp_vbl
   );
        
   color_correct : linear2logarithmic
   port map (
      lin => fifo_dataout,
      log => api_colordata
   );  
   
   PC : UART
   port map (
      clkx8      => clk24M,
      reset      => panel_reset,
      rx         => uart_rx,
      --
      datain     => uart_datain,
      datain_clk => uart_dataclk
   );
   
   PCFIFO : FIFO 
   port map (
      i_reset   => panel_reset,
      i_clk     => clk180M,
      i_wen     => fifo_wen,
      i_ren     => fifo_ren,
      i_data    => fifo_datain,
      o_data    => fifo_dataout,
      o_full    => open,
      o_empty   => fifo_empty
   );
   
   dsp_latch <= s_dsp_latch;

   tst_fifo_ren    <= fifo_ren;
   tst_fifo_dataout <= fifo_dataout;
   tst_fifo_wen    <= fifo_wen;
   tst_ram_wr_clk  <= ram_wr_clk;
   tst_ram_wr_data <= ram_wr_colorbits;
   tst_ram_wr_mask <= std_logic_vector(ram_wr_mask);
   tst_color_update_done <= color_update_done;
   
   p_dispread : process (reset, ram_rd_data)   
   begin
      panel_reset <= reset;
      dsp_rgbs    <= ram_rd_data;
   end process;
   
   p_clocks: process (clk180M)
   variable clk_scaler  : integer := 0;
   variable uart_scaler : integer := 0;
   begin
      if rising_edge(clk180M) then
                 
         case clk_scaler is
         when 0 => 
            clk60M <= '1';                      
            clk_scaler := 1;
         when 1 => 
            clk60M <= '0';
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
            clk24M <= '1';
            uart_scaler := uart_scaler + 1;
         when 4 => 
         when 12 => 
            clk24M <= '0';
            uart_scaler := uart_scaler + 1;
         when 15 =>
            uart_scaler := 0;
         when others =>         
            uart_scaler := uart_scaler + 1;
         end case;
      end if;
   end process;

   p_sram: process(clk180M)
   variable sramwritestep     : integer range 0 to 7 := 0;
   variable wrmask            : unsigned(11 downto 0);
   variable wraddress         : unsigned(14 downto 0);
   variable wrcolorsbits      : std_logic_vector(7 downto 0);
   variable wrcolorbit        : std_logic_vector(7 downto 0);
   variable tmpdata           : unsigned(11 downto 0);
   variable writing           : std_logic := '0';
   
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
   -- 256*32*68 clocks.
   -- With a max clock of 30MHz this means a single screen renders in (256*32*68)/30E6 = 18.3ms.
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
   -- So writing the R, G or B channel of a single pixel involves 8 updates
   -- in memory. Such an update is done by setting the ram_wr_mask, ram_wr_addr and ram_wr_clk.
   begin    
      if rising_edge(clk180M) then      
         sram_cs <= '0';
         sram_oe <= '0';
         color_update_done <= '0';
         
         if ram_wr_clk = '1' then -- will remain high only 1 clock
            -- initialize new color write sequence.
            wraddress     := ram_wr_addr;
            wrcolorsbits  := ram_wr_colorbits;
            wrmask        := ram_wr_mask;
            wrcolorbit    := "10000000";
            sramwritestep := 0;              
            writing       := '1';
         elsif dsp_latch = '0' then           
            -- read external display during a refresh cycle.
            sram_wr     <= '1';
            sram_data   <= (others => 'Z');               
            sram_addr   <= ram_rd_addr;
            ram_rd_data <= sram_data;         
            
         elsif writing = '1' then
            -- update external display ram during display idle periods.                       
            case sramwritestep is
            when 0 => -- set address to read displaybits and optionally clock-in updated bits
               if wrcolorbit = "00000000" then
                  -- all bits written, we're done
                  writing         :='0';
                  color_update_done <= '1';
               else
                  sram_wr   <= '1';
                  sram_data <= (others => 'Z');               
                  sram_addr <= std_logic_vector(wraddress);               
               end if;
            when 2 => -- clock-in current displaybit and prepare for update
               tmpdata := unsigned(sram_data);
               tmpdata := tmpdata and not wrmask;
               if ((wrcolorsbits and wrcolorbit) /= "00000000") then
                  tmpdata := tmpdata or wrmask;
               end if;            
               sram_data <= std_logic_vector(tmpdata);
               sram_wr <= '0'; 
               -- prepare for next bit
               wrcolorbit := '0' & wrcolorbit(7 downto 1);
               wraddress := wraddress + 16#0800#;
            when others =>
            end case;                   
            sramwritestep := sramwritestep + 1;     
            if sramwritestep = 4 then
               sramwritestep := 0;
            end if;
         end if;
      end if;
   end process;
   
   p_receive_api: process(reset, clk180M)
   variable rcv_data         : std_logic_vector (7 downto 0);
   variable rcv_dataclk      : std_logic;
   variable last_rcv_dataclk : std_logic;
   variable init_fill_delay  : integer;
   variable init_fill_state  : integer; 
   begin
      if reset = '1' then
         rcv_dataclk      := '0';
         last_rcv_dataclk := '0';
         init_fill_state  := 0;
         init_fill_delay  := 1;
      
      elsif rising_edge(clk180M) then      
         if init_fill_state = 15 then
            rcv_data    := uart_datain;
            rcv_dataclk := uart_dataclk;
         else
            init_fill_delay := init_fill_delay - 1;
            if init_fill_delay = 0 then
               init_fill_delay := 3;
               case init_fill_state is
               when 0 => rcv_data := X"01"; -- start address
               when 1 => rcv_data := X"00";
               when 2 => rcv_data := X"01";
               when 3 => rcv_data := X"02"; -- pixel count
               when 4 => rcv_data := X"00";
               when 5 => rcv_data := X"03";
               when 6 => rcv_data := X"FF"; -- R
               when 7 => rcv_data := X"00"; -- G
               when 8 => rcv_data := X"00"; -- B
               when 9 => rcv_data := X"00";
               when 10 => rcv_data := X"FF";
               when 11 => rcv_data := X"00";
               when 12 => rcv_data := X"00";
               when 13 => rcv_data := X"00";
               when 14 => rcv_data := X"FF";
               when others => rcv_data := X"00";
               end case;
               init_fill_state := init_fill_state + 1;
               rcv_dataclk := '1';
            else
               rcv_dataclk := '0';
            end if;
         end if;
         
         -- write received data into fifo.
         if rcv_dataclk = '1' and last_rcv_dataclk = '0' then
            fifo_datain <= rcv_data;
            fifo_wen    <= rcv_dataclk;
         else
            fifo_wen    <= '0';
         end if;
         last_rcv_dataclk := rcv_dataclk;
      end if;   
   end process;
      
   p_handleapi: process (reset, clk180M)
   type T_APISTATE is ( START, ADDRHI, ADDRLO, PIXCOUNTHI, PIXCOUNTLO, WRITE_R, WRITE_G, WRITE_B ); 
   type T_READFIFOSTATE is ( PREPARE, GET, EXECUTE, WRITING_COLOR ); 
   variable readfifostate       : T_READFIFOSTATE;
   variable apistate            : T_APISTATE;
   variable pixel_count         : std_logic_vector(12 downto 0);
   variable pixel_addr          : std_logic_vector(12 downto 0);
   variable mapped_address      : unsigned(13 downto 0);      
   begin   
      if reset = '1' then
         readfifostate   := PREPARE;
         apistate        := START;  
         fifo_ren        <= '0';
         ram_wr_mask     <= (others => '0');    
         ram_wr_addr     <= (others => '0');               
      
      elsif rising_edge(clk180M) then
         -- ram_wr_mask indicates that a colorbyte is currently being written (this
         -- takes 8 read-modify-write cycles. It is cleared by the p_sram when completed.
         ram_wr_clk <= '0';         
         case readfifostate is
         when PREPARE =>
            if fifo_empty = '0' then
               fifo_ren      <= '1';
               readfifostate := GET;
            end if;
         when GET =>
            fifo_ren      <= '0';
            readfifostate := EXECUTE;
            ram_wr_colorbits <= api_colordata;   -- fifodata lin->log in case it is a colorbyte
         when EXECUTE =>
            case apistate is
            when START =>            
               if (unsigned(fifo_dataout) = X"01") then
                  apistate := ADDRHI;
               elsif (unsigned(fifo_dataout) = X"02") then
                  apistate := PIXCOUNTHI;
               end if;
               readfifostate := PREPARE;
            when ADDRHI =>
               pixel_addr(12 downto 8) := fifo_dataout(4 downto 0);
               apistate := ADDRLO;
               readfifostate := PREPARE;
            when ADDRLO =>
               pixel_addr(7 downto 0) := fifo_dataout;
               apistate := START;            
               readfifostate := PREPARE;
            when PIXCOUNTHI =>
               pixel_count(12 downto 8) := fifo_dataout(4 downto 0);
               apistate := PIXCOUNTLO;            
               readfifostate := PREPARE;
            when PIXCOUNTLO =>
               pixel_count(7 downto 0) := fifo_dataout;
               apistate := WRITE_R;            
               ram_wr_addr <= unsigned(pixel_addr(11 downto 7) & pixel_addr(5 downto 0) & "0000");
               readfifostate := PREPARE;
            when WRITE_R =>
               case std_logic_vector'(pixel_addr(6) & pixel_addr(12)) is
               when "00" =>
                   ram_wr_mask <= "000000000001";
               when "01" =>
                   ram_wr_mask <= "000000001000";
               when "10" =>
                   ram_wr_mask <= "000001000000";
               when "11" =>
                   ram_wr_mask <= "001000000000";
               when others =>
               end case;
               ram_wr_clk <= '1';
               apistate := WRITE_G;
               readfifostate := WRITING_COLOR;
            when WRITE_G =>
               case std_logic_vector'(pixel_addr(6) & pixel_addr(12)) is
               when "00" =>
                   ram_wr_mask <= "000000000010";
               when "01" =>
                   ram_wr_mask <= "000000010000";
               when "10" =>
                   ram_wr_mask <= "000010000000";
               when "11" =>
                   ram_wr_mask <= "010000000000";
               when others =>
               end case;
               ram_wr_clk <= '1';
               apistate := WRITE_B;
               readfifostate := WRITING_COLOR;
            when WRITE_B =>
               case std_logic_vector'(pixel_addr(6) & pixel_addr(12)) is
               when "00" =>
                   ram_wr_mask <= "000000000100";
               when "01" =>
                   ram_wr_mask <= "000000100000";
               when "10" =>
                   ram_wr_mask <= "000100000000";
               when "11" =>
                   ram_wr_mask <= "100000000000";
               when others =>
               end case;
               ram_wr_clk <= '1';
               ram_wr_addr <= ram_wr_addr + 1;
               pixel_count := std_logic_vector(unsigned(pixel_count) - 1);
               if (pixel_count = "0000000000000") then
                  apistate := START;
               else
                  apistate := WRITE_R;
               end if;
               readfifostate := WRITING_COLOR;
            end case;   
         when WRITING_COLOR =>
            if color_update_done = '1' then
               readfifostate := PREPARE;
            end if;
         end case;      
      end if;      
   end process;
   
end architecture ledpanel_controller_arch;            