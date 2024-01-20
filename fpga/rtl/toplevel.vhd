library IEEE;
use IEEE.STD_LOGIC_1164.all;
use IEEE.NUMERIC_STD.all;

-- Entity that combines the ledpanel_controller with the PLL
--- This allows us to simulate the ledpanel_controller isolated from the PLL.
entity toplevel is
   port (    
      i_clk100M     : in std_logic;
      i_reset_n     : in std_logic;
      i_uart_rx     : in std_logic;
      -- --
      o_uart_tx     : out std_logic;
      o_dsp_clk     : out std_logic;
      o_dsp_latch   : out std_logic;
      o_dsp_oe_n    : out std_logic;
      o_dsp_addr    : out std_logic_vector (4 downto 0);
      o_dsp_rgbs    : out std_logic_vector (11 downto 0);
      o_led1        : out std_logic;
      o_led2        : out std_logic;
      -- sram pins
      o_sram_oe     : out std_logic;
      o_sram_wr     : out std_logic;
      o_sram_cs     : out std_logic;
      o_sram_addr   : out std_logic_vector(14 downto 0);
      io_sram_data  : inout std_logic_vector(11 downto 0)
   );
end entity toplevel;

architecture toplevel_arch of toplevel is

   component pll
   port (
      REFERENCECLK: in std_logic;
      RESET:        in std_logic;
      --
      PLLOUTCORE:   out std_logic;
      PLLOUTGLOBAL: out std_logic
   );
   end component;
   
   component ledpanel_controller
   port (    
      i_clk60M      : in std_logic;
      i_reset_n     : in std_logic;
      i_uart_rx     : in std_logic;
      --
      o_uart_tx     : out std_logic;
      o_dsp_clk     : out std_logic;
      o_dsp_latch   : out std_logic;
      o_dsp_oe_n    : out std_logic;
      o_dsp_addr    : out std_logic_vector (4 downto 0);
      o_dsp_rgbs    : out std_logic_vector (11 downto 0);
      o_dsp_vbl     : out std_logic;
      -- sram pins
      o_sram_oe     : out std_logic;
      o_sram_wr     : out std_logic;
      o_sram_cs     : out std_logic;
      o_sram_addr   : out std_logic_vector(13 downto 0);
      io_sram_data  : inout std_logic_vector(11 downto 0);
      --
      o_test        : out std_logic
   );
   end component;

   signal s_clk60M     : std_logic;
   signal s_panel_addr : std_logic_vector(13 downto 0);
     
begin  
   clock_generator : pll
   port map (
      REFERENCECLK  => i_clk100M,
      RESET         => i_reset_n,
      --
      PLLOUTGLOBAL  => s_clk60M,
      plloutcore    => open
   );
      
   panel_controller : ledpanel_controller
   port map (
      i_clk60M      => s_clk60M,
      i_reset_n     => i_reset_n,
      i_uart_rx     => i_uart_rx,
      o_uart_tx     => o_uart_tx,

      o_dsp_clk     => o_dsp_clk,
      o_dsp_latch   => o_dsp_latch,
      o_dsp_oe_n    => o_dsp_oe_n,
      o_dsp_addr    => o_dsp_addr,
      o_dsp_rgbs    => o_dsp_rgbs,
      
      o_sram_oe     => o_sram_oe,
      o_sram_wr     => o_sram_wr,
      o_sram_cs     => o_sram_cs,
      o_sram_addr   => s_panel_addr,
      io_sram_data  => io_sram_data,
      
      o_test       => o_led2
   ); 

   o_sram_addr <= "0" & s_panel_addr;
   o_led1      <= i_uart_rx;

end architecture toplevel_arch;
            
            
            
            