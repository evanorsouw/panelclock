library IEEE;
use IEEE.STD_LOGIC_1164.all;
use IEEE.NUMERIC_STD.all;

entity ledpanel_controller is
   port (    
      clk12M    : in std_logic;
      reset       : in std_logic;
      uart_rx   : in std_logic;
      --
      dsp_clk   : out std_logic;
      dsp_latch : out std_logic;
      dsp_oe    : out std_logic;
      dsp_addr  : out std_logic_vector (4 downto 0);
      dsp_r1    : out std_logic;
      dsp_g1    : out std_logic;
      dsp_b1    : out std_logic;
      dsp_r2    : out std_logic;
      dsp_g2    : out std_logic;
      dsp_b2    : out std_logic
   );
end entity ledpanel_controller;

architecture ledpanel_controller_arch of ledpanel_controller is

   component pll
   port (
      inclk0   : in std_logic := '0';
      --
      c0         : out std_logic
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
   signal color_r1        : std_logic;
   signal color_g1        : std_logic;
   signal color_b1        : std_logic;
   signal color_r2        : std_logic;
   signal color_g2        : std_logic;
   signal color_b2        : std_logic;
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
   signal wr_clk_r1       : std_logic;
   signal wr_clk_g1       : std_logic;
   signal wr_clk_b1       : std_logic;
   signal wr_clk_r2       : std_logic;
   signal wr_clk_g2       : std_logic;
   signal wr_clk_b2       : std_logic;
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
   
   ram_r1 : color_bitram 
   port map (
      rd_clock   => clk,
      rd_addr    => ram_rd_addr (13 downto 0),
      wr_clock   => wr_clk_r1,
      wr_addr    => ram_wr_addr (10 downto 0),
      wr_data    => ram_wr_data (7 downto 0),
      wr_en      => '1',
      --
      q          => color_r1
   );

   ram_g1 : color_bitram 
   port map (
      rd_clock   => clk,
      rd_addr    => ram_rd_addr (13 downto 0),
      wr_clock   => wr_clk_g1,
      wr_addr    => ram_wr_addr (10 downto 0),
      wr_data    => ram_wr_data (7 downto 0),
      wr_en      => '1',
      --
      q          => color_g1
   );

   ram_b1 : color_bitram 
   port map (
      rd_clock   => clk,
      rd_addr    => ram_rd_addr (13 downto 0),
      wr_clock   => wr_clk_b1,
      wr_addr    => ram_wr_addr (10 downto 0),
      wr_data    => ram_wr_data (7 downto 0),
      wr_en      => '1',
      --
      q          => color_b1
   );

   ram_r2 : color_bitram 
   port map (
      rd_clock   => clk,
      rd_addr    => ram_rd_addr (13 downto 0),
      wr_clock   => wr_clk_r2,
      wr_addr    => ram_wr_addr (10 downto 0),
      wr_data    => ram_wr_data (7 downto 0),
      wr_en      => '1',
      --
      q          => color_r2
   );

   ram_g2 : color_bitram 
   port map (
      rd_clock   => clk,
      rd_addr    => ram_rd_addr (13 downto 0),
      wr_clock   => wr_clk_g2,
      wr_addr    => ram_wr_addr (10 downto 0),
      wr_data    => ram_wr_data (7 downto 0),
      wr_en      => '1',
      --
      q          => color_g2
   );

   ram_b2 : color_bitram 
   port map (
      rd_clock   => clk,
      rd_addr    => ram_rd_addr (13 downto 0),
      wr_clock   => wr_clk_b2,
      wr_addr    => ram_wr_addr (10 downto 0),
      wr_data    => ram_wr_data (7 downto 0),
      wr_en      => '1',
      --
      q          => color_b2
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
      dsp_r1 <= color_r1;
      dsp_g1 <= color_g1;
      dsp_b1 <= color_b1;
      dsp_r2 <= color_r2;
      dsp_g2 <= color_g2;
      dsp_b2 <= color_b2;
      
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
      
            wr_clk_r1 <= '0';
            wr_clk_g1 <= '0';
            wr_clk_b1 <= '0';
            wr_clk_r2 <= '0';
            wr_clk_g2 <= '0';
            wr_clk_b2 <= '0';
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
               wr_clk_r1      <= not ram_wr_addr(11);
               wr_clk_r2      <= ram_wr_addr(11);
               state          := WRITE_G;
            when WRITE_G =>
               wr_clk_g1      <= not ram_wr_addr(11);
               wr_clk_g2      <= ram_wr_addr(11);
               state          := WRITE_B;
            when WRITE_B =>
               wr_clk_b1      <= not ram_wr_addr(11);
               wr_clk_b2      <= ram_wr_addr(11);
               write_address  <= write_address + 1;
               write_count    := write_count - 1;
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
            
            
            
            