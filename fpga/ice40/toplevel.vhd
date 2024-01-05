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
      o_dsp_clk     : out std_logic;
      o_dsp_latch   : out std_logic;
      o_dsp_oe      : out std_logic;
      o_dsp_addr    : out std_logic_vector (4 downto 0);
      o_dsp_rgbs    : out std_logic_vector (11 downto 0);
      o_test1       : out std_logic;
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
      i_clk120m     : in std_logic;
      i_reset_n     : in std_logic;
      i_uart_rx     : in std_logic;
      --
      o_dsp_clk     : out std_logic;
      o_dsp_latch   : out std_logic;
      o_dsp_oe      : out std_logic;
      o_dsp_addr    : out std_logic_vector (4 downto 0);
      o_dsp_rgbs    : out std_logic_vector (11 downto 0);
      o_test1       : out std_logic;
      -- sram pins
      o_sram_oe     : out std_logic;
      o_sram_wr     : out std_logic;
      o_sram_cs     : out std_logic;
      o_sram_addr   : out std_logic_vector(14 downto 0);
      io_sram_data  : inout std_logic_vector(11 downto 0)
   );
   end component;

   signal s_clk120M : std_logic;
   
begin  
   clock_generator : pll
   port map (
      REFERENCECLK   => i_clk100M,
      RESET          => i_reset_n,
      --
      PLLOUTGLOBAL   => s_clk120M,
      plloutcore     => open
   );
      
   panel_controller : ledpanel_controller
   port map (
      i_clk120M     => s_clk120M,
      i_reset_n     => i_reset_n,
      i_uart_rx     => i_uart_rx,

      o_dsp_clk     => o_dsp_clk,
      o_dsp_latch   => o_dsp_latch,
      o_dsp_oe      => o_dsp_oe,
      o_dsp_addr    => o_dsp_addr,
      o_dsp_rgbs    => o_dsp_rgbs,
      o_test1       => o_test1,
      
      o_sram_oe     => o_sram_oe,
      o_sram_wr     => o_sram_wr,
      o_sram_cs     => o_sram_cs,
      o_sram_addr   => o_sram_addr,
      io_sram_data  => io_sram_data
   );  
      
end architecture toplevel_arch;
            
            
            
            