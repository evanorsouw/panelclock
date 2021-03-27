library IEEE;
use IEEE.STD_LOGIC_1164.all;
use IEEE.NUMERIC_STD.all;

entity ledpanel_controller is
	port ( 	
		clk12M    : in std_logic;
		reset		 : in std_logic;
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
		inclk0	: in std_logic := '0';
		--
		c0			: out std_logic
	);
	end component;
	
	component FM6124
	port ( 	
		clk       : in std_logic;
		reset     : in std_logic;
		data      : in std_logic_vector (5 downto 0);
		--
		addr      : out std_logic_vector (11 downto 0);
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
   end component;
	
	signal panel_reset : std_logic := '0';
	signal clk         : std_logic;
	signal ram_addr    : std_logic_vector (11 downto 0);
	signal ram_data    : std_logic_vector (5 downto 0);
	
begin
	LEDChip : FM6124
	port map (
		clk    => clk,
		reset  => panel_reset,
		data   => ram_data,
		--
		addr      => ram_addr,
		dsp_clk   => dsp_clk,
		dsp_latch => dsp_latch,
		dsp_addr  => dsp_addr,
		dsp_oe    => dsp_oe,
		dsp_r1    => dsp_r1,
		dsp_g1    => dsp_g1,
		dsp_b1    => dsp_b1,
		dsp_r2    => dsp_r2,
		dsp_g2    => dsp_g2,
		dsp_b2    => dsp_b2
	);
	
	ClockGenerator : pll
	port map (
		inclk0  => clk12M,
		--
		c0 => clk
	);
	
	--B2 G2 R1 B1 G1 R2
	Reset_Proc: process (clk,reset)
	begin
		panel_reset <= not reset;
		if rising_edge(clk) then
			if (unsigned(ram_addr) = 0 or unsigned(ram_addr) = 127) then
				ram_data <= "000111";
			elsif (unsigned(ram_addr) = 3968 or unsigned(ram_addr) = 4095) then
				ram_data <= "111000";
			else
				ram_data <= "000000";
			end if;
		end if;
	end process;
	
end architecture ledpanel_controller_arch;
				
				
				
				