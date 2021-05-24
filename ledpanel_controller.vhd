library IEEE;
use IEEE.STD_LOGIC_1164.all;
use IEEE.NUMERIC_STD.all;

entity ledpanel_controller is
   port (    
      clk12M      : in std_logic;
      reset       : in std_logic;
      uart_rx     : in std_logic;
      --
      dsp_clk     : out std_logic;
      dsp_latch   : out std_logic;
      dsp_oe      : out std_logic;
      dsp_addr    : out std_logic_vector (4 downto 0);
      dsp_channel : out std_logic_vector (11 downto 0)
   );
end entity ledpanel_controller;

architecture ledpanel_controller_arch of ledpanel_controller is

   component pll
   port (
      inclk0    : in std_logic := '0';
      --
      c0        : out std_logic
   );
   end component;
   
   component FM6124
   port (    
      clk       : in std_logic;
      reset     : in std_logic;
      --
      addr      : out std_logic_vector (13 downto 0);
      dsp_clk   : out std_logic;
      dsp_latch : out std_logic;
      dsp_oe    : out std_logic;
      dsp_addr  : out std_logic_vector (4 downto 0);
      vbl       : out std_logic
   );
   end component;
   
   component color_bitram
   port (    
      rd_clock  : in std_logic;
      rd_addr   : in std_logic_vector (13 downto 0);
      wr_clock  : in std_logic;
      wr_addr   : in std_logic_vector (10 downto 0);
      wr_data   : in std_logic_vector (7 downto 0);
      wr_en     : in std_logic;
      --
      q         : out std_logic
   );
   end component;
   
   component linear2logarithmic
   port (    
      lin    : in std_logic_vector (7 downto 0);
      --
      log      : out std_logic_vector (7 downto 0)
   );
   end component;   

   component brightness_control 
    port (
        valuein:	  in  std_logic_vector(7 downto 0);
        brightness: in std_logic_vector (7 downto 0);
        --
        valueout:	  out std_logic_vector(7 downto 0)
    );
   end component;   

   component UART
   PORT
   (
      clkx8         : in std_logic;
      reset         : in std_logic;
      rx            : in std_logic;
      datain        : out std_logic_vector (7 downto 0);
      datain_clk    : out std_logic
   );
   end component;
   
   signal panel_reset     : std_logic := '0';
   signal clk             : std_logic;
   signal ram_rd_addr     : std_logic_vector (13 downto 0);
   signal color_chan      : std_logic_vector (11 downto 0);
   signal color_wr_clk    : std_logic_vector (11 downto 0);
   signal uart_clk        : std_logic;
   signal uart_delay      : unsigned (15 downto 0);
   signal uart_datain     : std_logic_vector (7 downto 0);
   signal brightness_byte : std_logic_vector (7 downto 0);
   signal color_byte      : std_logic_vector (7 downto 0);
   signal uart_dataclk    : std_logic;
   signal ram_wr_addr     : std_logic_vector(15 downto 0);
   signal ram_wr_data     : std_logic_vector (7 downto 0);
   signal visible_page    : std_logic;
   signal vbl             : std_logic;
   signal dsp_base_addr   : std_logic_vector (15 downto 0);
   signal write_address   : unsigned (15 downto 0);
   signal brightness      : std_logic_vector (7 downto 0) := "11111111";

begin
   LEDChip : FM6124
   port map (
      clk       => clk,
      reset     => panel_reset,
      --
      addr      => ram_rd_addr(13 downto 0),
      dsp_clk   => dsp_clk,
      dsp_latch => dsp_latch,
      dsp_addr  => dsp_addr,
      dsp_oe    => dsp_oe,
      vbl       => vbl
   );
   
   ClockGenerator : pll
   port map (
      inclk0  => clk12M,
      --
      c0 => clk
   );
   
   ram_0 : color_bitram 
   port map (
      rd_clock   => clk,
      rd_addr    => ram_rd_addr (13 downto 0),
      wr_clock   => color_wr_clk(0),
      wr_addr    => ram_wr_addr (10 downto 0),
      wr_data    => ram_wr_data (7 downto 0),
      wr_en      => '1',
      --
      q          => color_chan(0)
   );

   ram_1 : color_bitram 
   port map (
      rd_clock   => clk,
      rd_addr    => ram_rd_addr (13 downto 0),
      wr_clock   => color_wr_clk(1),
      wr_addr    => ram_wr_addr (10 downto 0),
      wr_data    => ram_wr_data (7 downto 0),
      wr_en      => '1',
      --
      q          => color_chan(1)
   );

   ram_2 : color_bitram 
   port map (
      rd_clock   => clk,
      rd_addr    => ram_rd_addr (13 downto 0),
      wr_clock   => color_wr_clk(2),
      wr_addr    => ram_wr_addr (10 downto 0),
      wr_data    => ram_wr_data (7 downto 0),
      wr_en      => '1',
      --
      q          => color_chan(2)
   );

   ram_3 : color_bitram 
   port map (
      rd_clock   => clk,
      rd_addr    => ram_rd_addr (13 downto 0),
      wr_clock   => color_wr_clk(3),
      wr_addr    => ram_wr_addr (10 downto 0),
      wr_data    => ram_wr_data (7 downto 0),
      wr_en      => '1',
      --
      q          => color_chan(3)
   );

   ram_4 : color_bitram 
   port map (
      rd_clock   => clk,
      rd_addr    => ram_rd_addr (13 downto 0),
      wr_clock   => color_wr_clk(4),
      wr_addr    => ram_wr_addr (10 downto 0),
      wr_data    => ram_wr_data (7 downto 0),
      wr_en      => '1',
      --
      q          => color_chan(4)
   );

   ram_5 : color_bitram 
   port map (
      rd_clock   => clk,
      rd_addr    => ram_rd_addr (13 downto 0),
      wr_clock   => color_wr_clk(5),
      wr_addr    => ram_wr_addr (10 downto 0),
      wr_data    => ram_wr_data (7 downto 0),
      wr_en      => '1',
      --
      q          => color_chan(5)
   );
   
   ram_6 : color_bitram 
   port map (
      rd_clock   => clk,
      rd_addr    => ram_rd_addr (13 downto 0),
      wr_clock   => color_wr_clk(6),
      wr_addr    => ram_wr_addr (10 downto 0),
      wr_data    => ram_wr_data (7 downto 0),
      wr_en      => '1',
      --
      q          => color_chan(6)
   );

   ram_7 : color_bitram 
   port map (
      rd_clock   => clk,
      rd_addr    => ram_rd_addr (13 downto 0),
      wr_clock   => color_wr_clk(7),
      wr_addr    => ram_wr_addr (10 downto 0),
      wr_data    => ram_wr_data (7 downto 0),
      wr_en      => '1',
      --
      q          => color_chan(7)
   );

   ram_8 : color_bitram 
   port map (
      rd_clock   => clk,
      rd_addr    => ram_rd_addr (13 downto 0),
      wr_clock   => color_wr_clk(8),
      wr_addr    => ram_wr_addr (10 downto 0),
      wr_data    => ram_wr_data (7 downto 0),
      wr_en      => '1',
      --
      q          => color_chan(8)
   );

   ram_9 : color_bitram 
   port map (
      rd_clock   => clk,
      rd_addr    => ram_rd_addr (13 downto 0),
      wr_clock   => color_wr_clk(9),
      wr_addr    => ram_wr_addr (10 downto 0),
      wr_data    => ram_wr_data (7 downto 0),
      wr_en      => '1',
      --
      q          => color_chan(9)
   );

   ram_10 : color_bitram 
   port map (
      rd_clock   => clk,
      rd_addr    => ram_rd_addr (13 downto 0),
      wr_clock   => color_wr_clk(10),
      wr_addr    => ram_wr_addr (10 downto 0),
      wr_data    => ram_wr_data (7 downto 0),
      wr_en      => '1',
      --
      q          => color_chan(10)
   );

   ram_11 : color_bitram 
   port map (
      rd_clock   => clk,
      rd_addr    => ram_rd_addr (13 downto 0),
      wr_clock   => color_wr_clk(11),
      wr_addr    => ram_wr_addr (10 downto 0),
      wr_data    => ram_wr_data (7 downto 0),
      wr_en      => '1',
      --
      q          => color_chan(11)
   );
   
   brightness_correct : brightness_control
   port map (
      valuein    => uart_datain,
      brightness => brightness,
      valueout   => brightness_byte
   );
    
   color_correct : linear2logarithmic
   port map (
      lin => brightness_byte,
      log => color_byte
   );  
   
   PC : UART
   port map (
      clkx8      => uart_clk,
      reset      => panel_reset,
      rx         => uart_rx,
      --
      datain     => uart_datain,
      datain_clk => uart_dataclk
   );
   
   p_dispread : process (reset, clk)   
   begin
      panel_reset <= not reset;
      dsp_channel <= color_chan;
   end process;
   
   p_uartclk: process (clk)
   variable counter : unsigned (8 downto 0);
   begin
      if rising_edge(clk) then
         uart_clk <= not uart_clk;
      end if;
   end process;

   
   p_uartapi: process (clk)
   type T_APISTATE is ( START, DSPADDRHI, DSPADDRLO, ADDRHI, ADDRLO, PIXCOUNT, WRITE_R, WRITE_G, WRITE_B, BRIGHTNESS ); 
   variable state         : T_APISTATE;
   variable write_count   : unsigned (7 downto 0);
   variable uart_clock    : std_logic_vector (2 downto 0);
      
   begin
   
      if rising_edge(clk) then
      
         uart_clock := uart_clock (1 downto 0) & uart_dataclk;
         if (uart_clock(2) = '0' and uart_clock(1) = '1') then
      
            color_wr_clk <= "000000000000";
            case state is
            when START =>            
               if (unsigned(uart_datain) = X"01") then
                  state := ADDRHI;
               elsif (unsigned(uart_datain) = X"02") then
                  state := PIXCOUNT;
               elsif (unsigned(uart_datain) = X"03") then
                  state := DSPADDRHI;
               elsif (unsigned(uart_datain) = X"04") then
                  state := BRIGHTNESS;
               end if;
            when DSPADDRHI =>
               dsp_base_addr(15 downto 8) <= std_logic_vector(uart_datain);
               state := DSPADDRLO;
            when DSPADDRLO =>
               dsp_base_addr(7 downto 0) <= std_logic_vector(uart_datain);
               state := START;
            when ADDRHI =>
               write_address(15 downto 8) <= unsigned(uart_datain);
               state := ADDRLO;
            when ADDRLO =>
               write_address(7 downto 0) <= unsigned(uart_datain);
               state := START;            
            when PIXCOUNT =>
               write_count := unsigned(uart_datain);
               state := WRITE_R;            
            when WRITE_R =>
               case ram_wr_addr(12 downto 11) is
                  when "00" => color_wr_clk(0) <= '1';
                  when "01" => color_wr_clk(3) <= '1';
                  when "10" => color_wr_clk(6) <= '1';
                  when "11" => color_wr_clk(9) <= '1';
               end case;
               state           := WRITE_G;
            when WRITE_G =>
               case ram_wr_addr(12 downto 11) is
                  when "00" => color_wr_clk(1) <= '1';
                  when "01" => color_wr_clk(4) <= '1';
                  when "10" => color_wr_clk(7) <= '1';
                  when "11" => color_wr_clk(10) <= '1';
               end case;
               state           := WRITE_B;
            when WRITE_B =>
               case ram_wr_addr(12 downto 11) is
                  when "00" => color_wr_clk(2) <= '1';
                  when "01" => color_wr_clk(5) <= '1';
                  when "10" => color_wr_clk(8) <= '1';
                  when "11" => color_wr_clk(11) <= '1';
               end case;
               write_address   <= write_address + 1;
               write_count     := write_count - 1;
               if (write_count = 0) then
                  state := START;
               else
                  state := WRITE_R;
               end if;
            when BRIGHTNESS =>
               brightness <= uart_datain;
               state := START;
            when others =>
               state := START;
            end case;      
         end if;
         
         ram_wr_addr  <= std_logic_vector(write_address);
         ram_wr_data  <= std_logic_vector(color_byte);
            
      end if;
      
   end process;
   
end architecture ledpanel_controller_arch;
            
            
            
            