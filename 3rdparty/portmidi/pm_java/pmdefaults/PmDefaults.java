// PmDefaults -- a small application to set PortMIDI default input/output

package pmdefaults;
import javax.swing.SwingUtilities;

public class PmDefaults {
    public static void main(String[] args) {
        System.out.println("starting main");
        SwingUtilities.invokeLater(new Runnable() {
                public void run() {
                    new PmDefaultsFrame("PortMIDI Setup");
                }
            });
    }
}
