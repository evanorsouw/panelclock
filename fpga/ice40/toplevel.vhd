library IEEE;
use IEEE.STD_LOGIC_1164.all;
use IEEE.NUMERIC_STD.all;

-- Entity that combines the ledpanel_controller with the PLL
--- This allows us to simulate the ledpanel_controller isolated from the PLL.
entity toplevel is
   port (    
      clk100M     : in std_logic;
      reset       : in std_logic;
      uart_rx     : in std_logic;
      --
      dsp_clk     : out std_logic;
      dsp_latch   : out std_logic;
      dsp_oe      : out std_logic;
      dsp_addr    : out std_logic_vector (4 downto 0);
      dsp_rgbs    : out std_logic_vector (11 downto 0);
      test1       : out std_logic;
      -- sram pins
      sram_oe     : out std_logic;
      sram_wr     : out std_logic;
      sram_cs     : out std_logic;
      sram_addr   : out std_logic_vector(14 downto 0);
      sram_data   : inout std_logic_vector(11 downto 0)
   );
end entity toplevel;

architecture toplevel_arch of toplevel is

   component pll
   port (
      REFERENCECLK: in std_logic;
      RESET: in std_logic;
      --
      PLLOUTCORE: out std_logic;
      PLLOUTGLOBAL: out std_logic
   );
   end component;
   
   component ledpanel_controller
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
      test1       : out std_logic;
      -- sram pins
      sram_oe     : out std_logic;
      sram_wr     : out std_logic;
      sram_cs     : out std_logic;
      sram_addr   : out std_logic_vector(14 downto 0);
      sram_data   : inout std_logic_vector(11 downto 0)
   );
   end component;

   signal clk180M : std_logic;

begin  
   clock_generator : pll
   port map (
      REFERENCECLK  => clk100M,
      RESET => reset,
      --
      PLLOUTGLOBAL => clk180M,
      plloutcore => open
   );
      
   panel_controller : ledpanel_controller
   port map (
      clk180M     => clk180M,
      reset       => reset,
      uart_rx     => uart_rx,

      dsp_clk     => dsp_clk,
      dsp_latch   => dsp_latch,
      dsp_oe      => dsp_oe,
      dsp_addr    => dsp_addr,
      dsp_rgbs    => dsp_rgbs,
      test1       => test1,

      sram_oe     => sram_oe,
      sram_wr     => sram_wr,
      sram_cs     => sram_cs,
      sram_addr   => sram_addr,
      sram_data   => sram_data
   );  
      

end architecture toplevel_arch;
            
            
            
            