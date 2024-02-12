library IEEE;
use IEEE.STD_LOGIC_1164.all;
use IEEE.NUMERIC_STD.all;

entity cmd_screen is
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
end entity cmd_screen;

-- pixel commands
-- 0x09,0bWWWWRRRR - select screen for display (R) and writing (W)
architecture cmd_screen_arch of cmd_screen is

   type T_STATE is ( CMD, ARG_BASENO );

   signal s_x         : unsigned(7 downto 0);
   signal s_y         : unsigned(7 downto 0);
   signal s_write_clk : std_logic;
   signal s_state     : T_STATE;
   signal s_brush     : std_logic_vector(23 downto 0);
   signal s_alpha     : unsigned(5 downto 0);

begin
   cmd_screen: process (i_reset_n, i_clk)
   variable v_red : unsigned(13 downto 0);
   variable v_grn : unsigned(13 downto 0);
   variable v_blu : unsigned(13 downto 0);
   begin
      if i_reset_n = '0' then
         s_state          <= CMD;
         o_busy           <= '0';
         o_need_more_data <= '0';
         o_displayscreen  <= "00";
         o_writescreen    <= "00";
         
      elsif rising_edge(i_clk) then

         case s_state is
         when CMD =>
            if i_data_rdy = '1' then
               if i_data = X"09" then
                  o_busy <= '1';
                  o_need_more_data <= '1';
                  s_state <= ARG_BASENO;
               end if;
            end if;
         when ARG_BASENO =>
            if i_data_rdy = '1' then
               o_displayscreen <= i_data(3 downto 0);
               o_writescreen <= i_data(7 downto 4);
               o_busy <= '0';
               o_need_more_data <= '0';
               s_state <= CMD;
            end if;
         end case;
      end if;

   end process;
end architecture cmd_screen_arch;



